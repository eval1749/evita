// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/layout/line/root_inline_box_cache.h"

#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/text/buffer.h"
#include "evita/text/static_range.h"

namespace layout {

namespace {
bool IsWrappedLine(const RootInlineBox& line) {
  return line.boxes().back()->as<InlineMarkerBox>()->marker_name() ==
         TextMarker::LineWrap;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
RootInlineBoxCache::RootInlineBoxCache(const text::Buffer* buffer)
    : buffer_(buffer), zoom_(0.0f) {
  UI_DOM_AUTO_LOCK_SCOPE();
  const_cast<text::Buffer*>(buffer_)->AddObserver(this);
}

RootInlineBoxCache::~RootInlineBoxCache() {
  UI_DOM_AUTO_LOCK_SCOPE();
  const_cast<text::Buffer*>(buffer_)->RemoveObserver(this);
}

RootInlineBox* RootInlineBoxCache::FindLine(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return nullptr;
  const auto& it = lines_.lower_bound(offset);
  if (it == lines_.end()) {
    const auto& last = lines_.rbegin()->second;
    if (offset < last->text_start() || offset >= last->text_end())
      return nullptr;
    return last.get();
  }
  if (it->first == offset)
    return it->second.get();
  if (it == lines_.begin())
    return nullptr;
  const auto& line = std::prev(it)->second;
  if (offset < line->text_start() || offset >= line->text_end())
    return nullptr;
  return line.get();
}

void RootInlineBoxCache::Invalidate(const gfx::RectF& new_bounds,
                                    float new_zoom) {
  UI_ASSERT_DOM_LOCKED();
  if (zoom_ != new_zoom) {
    lines_.clear();
    bounds_ = new_bounds;
    zoom_ = new_zoom;
    return;
  }

  if (bounds_ == new_bounds)
    return;
  const auto is_width_changed = bounds_.width() != new_bounds.width();
  bounds_ = new_bounds;
  if (lines_.empty() || !is_width_changed)
    return;

  // Remove lines longer than |bounds_.width()|
  std::vector<text::Offset> dirty_offsets;
  for (const auto& pair : lines_) {
    const auto& line = pair.second;
    if (line->right() > new_bounds.right || !IsAfterNewline(line.get()) ||
        !IsEndWithNewline(line.get())) {
      dirty_offsets.push_back(line->text_start());
    }
  }
  for (auto offset : dirty_offsets) {
    const auto& it = lines_.find(offset);
    DCHECK(it != lines_.end());
    lines_.erase(it);
  }
}

bool RootInlineBoxCache::IsAfterNewline(const RootInlineBox* text_line) const {
  const auto start = text_line->text_start();
  return !start || buffer_->GetCharAt(start - text::OffsetDelta(1)) == '\n';
}

bool RootInlineBoxCache::IsDirty(const gfx::RectF& bounds, float zoom) const {
  if (zoom_ != zoom)
    return false;
  return bounds_ != bounds;
}

bool RootInlineBoxCache::IsEndWithNewline(
    const RootInlineBox* text_line) const {
  const auto end = text_line->text_end();
  if (end >= buffer_->GetEnd())
    return true;
  return buffer_->GetCharAt(end - text::OffsetDelta(1)) == '\n';
}

RootInlineBox* RootInlineBoxCache::Register(
    std::unique_ptr<RootInlineBox> line_ptr) {
  UI_ASSERT_DOM_LOCKED();
  const auto line = line_ptr.get();
  DCHECK_GE(line->text_end(), line->text_start());
  auto const present = lines_.find(line->text_start());
  if (present != lines_.end())
    lines_.erase(present);
  lines_.insert(std::make_pair(line->text_start(), std::move(line_ptr)));
#if _DEBUG
  const auto& it = lines_.find(line->text_start());
  if (it != lines_.begin())
    DCHECK_LE(std::prev(it)->second->text_end(), line->text_start());
  if (std::next(it) != lines_.end())
    DCHECK_GE(std::next(it)->second->text_start(), line->text_end());
#endif
  return line;
}

void RootInlineBoxCache::RelocateLines(text::Offset offset,
                                       text::OffsetDelta delta) {
  ASSERT_DOM_LOCKED();
  std::vector<RootInlineBox*> lines;
  for (;;) {
    const auto& it = lines_.lower_bound(offset);
    if (it == lines_.end())
      break;
    lines.push_back(it->second.release());
    lines_.erase(it);
  }
  for (auto line : lines) {
    line->UpdateTextStart(delta);
    DCHECK(lines_.find(line->text_start()) == lines_.end());
    lines_.insert(std::make_pair(line->text_start(),
                                 std::unique_ptr<RootInlineBox>(line)));
  }
}

void RootInlineBoxCache::RemoveOverwapLines(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return;
  const auto& last_line = lines_.rbegin()->second;
  if (last_line->text_end() <= range.start()) {
    // All cached lines end before |range.start()|.
    return;
  }
  const auto& it = lines_.lower_bound(range.start());
  if (it == lines_.end()) {
    // All lines start before |range.start()|.
    DCHECK_LT(range.start(), last_line->text_end());
    lines_.erase(lines_.find(lines_.rbegin()->first));
    if (lines_.empty())
      return;
    DCHECK_LE(lines_.rbegin()->second->text_end(), range.start());
    return;
  }
  // |start| line contains |offset| or after |offset|.
  const auto& start =
      it == lines_.begin() || std::prev(it)->second->text_end() <= range.start()
          ? it
          : std::prev(it);
  DCHECK_LT(range.start(), start->second->text_end());
  auto end = std::next(start);
  while (end != lines_.end()) {
    const auto line = end->second.get();
    if (line->text_start() >= range.end() && !line->IsContinuedLine())
      break;
    ++end;
  }
  lines_.erase(start, end);
}

// text::BufferMutationObserver
void RootInlineBoxCache::DidChangeStyle(const text::StaticRange& range) {
  TRACE_EVENT0("layout", "RootInlineBoxCache::DidChangeStyle");
  RemoveOverwapLines(range);
}

void RootInlineBoxCache::DidDeleteAt(const text::StaticRange& range) {
  TRACE_EVENT0("layout", "RootInlineBoxCache::DidDeleteAt");
  RemoveOverwapLines(range);
  RelocateLines(range.start(), text::OffsetDelta(-range.length().value()));
}

void RootInlineBoxCache::DidInsertBefore(const text::StaticRange& range) {
  TRACE_EVENT0("layout", "RootInlineBoxCache::DidInsertBefore");
  RemoveOverwapLines(range);
  RelocateLines(range.end(), range.length());
}

}  // namespace layout

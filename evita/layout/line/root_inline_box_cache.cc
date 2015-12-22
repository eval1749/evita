// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/layout/line/root_inline_box_cache.h"

#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/text/buffer.h"
#include "evita/text/static_range.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
RootInlineBoxCache::RootInlineBoxCache(const text::Buffer& buffer)
    : buffer_(buffer) {
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer_.AddObserver(this);
}

RootInlineBoxCache::~RootInlineBoxCache() {
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer_.RemoveObserver(this);
}

RootInlineBox* RootInlineBoxCache::FindLine(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return nullptr;
  auto it = lines_.lower_bound(offset);
  if (it == lines_.end())
    return nullptr;
  if (it->second->text_end() == offset) {
    ++it;
    if (it == lines_.end())
      return nullptr;
  }
  const auto line = it->second.get();
  return line->Contains(offset) ? line : nullptr;
}

RootInlineBox* RootInlineBoxCache::Insert(
    std::unique_ptr<RootInlineBox> line_ptr) {
  const auto line = line_ptr.get();
  DCHECK(lines_.find(line->text_end()) == lines_.end())
      << "We've already have RootInlineBox ends with " << line->text_end();
  lines_.insert(std::make_pair(line->text_end(), std::move(line_ptr)));
#if _DEBUG
  const auto& it = lines_.find(line->text_end());
  if (it != lines_.begin())
    DCHECK_LE(std::prev(it)->second->text_end(), line->text_start());
  if (std::next(it) != lines_.end())
    DCHECK_GE(std::next(it)->second->text_start(), line->text_end());
#endif
  return line;
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

  // Collect lines longer than |bounds_.width()|
  std::vector<text::Offset> dirty_lines;
  for (const auto& pair : lines_) {
    const auto& line = *pair.second;
    if (line.right() > new_bounds.right || line.IsContinuingLine() ||
        line.IsContinuedLine()) {
      dirty_lines.push_back(line.text_end());
    }
  }
  // Remove dirty lines
  for (auto offset : dirty_lines) {
    const auto& it = lines_.find(offset);
    DCHECK(it != lines_.end());
    lines_.erase(it);
  }
}

bool RootInlineBoxCache::IsDirty(const gfx::RectF& bounds, float zoom) const {
  if (zoom_ != zoom)
    return false;
  return bounds_ != bounds;
}

RootInlineBox* RootInlineBoxCache::Register(
    std::unique_ptr<RootInlineBox> line) {
  UI_ASSERT_DOM_LOCKED();
  DCHECK_GE(line->text_end(), line->text_start());
  auto const present = lines_.find(line->text_end());
  if (present != lines_.end())
    lines_.erase(present);
  return Insert(std::move(line));
}

void RootInlineBoxCache::RelocateLines(text::Offset offset,
                                       text::OffsetDelta delta) {
  ASSERT_DOM_LOCKED();
  std::vector<RootInlineBox*> lines;
  auto it = lines_.lower_bound(offset);
  if (it != lines_.end() && it->second->text_end() == offset)
    ++it;
  const auto start = it;
  while (it != lines_.end()) {
    lines.push_back(it->second.release());
    ++it;
  }
  lines_.erase(start, it);
  for (auto line : lines) {
    line->UpdateTextStart(delta);
    Insert(std::unique_ptr<RootInlineBox>(line));
  }
}

// Remove lines crossing |range|.
void RootInlineBoxCache::RemoveOverwapLines(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return;
  auto it = lines_.lower_bound(range.start());
  if (it == lines_.end()) {
    // All lines ends before |range.start()|
    return;
  }
  if (it->second->text_end() == range.start()) {
    // Skip line ends with |range.start()|.
    ++it;
  }
  const auto start = it;
  while (it != lines_.end() && it->second->text_start() < range.end())
    ++it;
  while (it != lines_.end() && it->second->IsContinuedLine())
    ++it;
  lines_.erase(start, it);
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

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/layout/line/root_inline_box_cache.h"

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
RootInlineBoxCache::RootInlineBoxCache(const text::Buffer* buffer)
    : buffer_(buffer), dirty_start_(text::Offset::Max()), zoom_(0.0f) {
  UI_DOM_AUTO_LOCK_SCOPE();
  const_cast<text::Buffer*>(buffer_)->AddObserver(this);
}

RootInlineBoxCache::~RootInlineBoxCache() {
  UI_DOM_AUTO_LOCK_SCOPE();
  const_cast<text::Buffer*>(buffer_)->RemoveObserver(this);
}

void RootInlineBoxCache::DidChangeBuffer(text::Offset offset) {
  ASSERT_DOM_LOCKED();
  dirty_start_ = std::min(dirty_start_, offset);
}

void RootInlineBoxCache::DidChangeStyle(const text::StaticRange& range) {
  DidChangeBuffer(range.start());
}

void RootInlineBoxCache::DidDeleteAt(const text::StaticRange& range) {
  DidChangeBuffer(range.start());
}

void RootInlineBoxCache::DidInsertBefore(const text::StaticRange& range) {
  DidChangeBuffer(range.start());
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
  if (zoom_ != new_zoom || !dirty_start_) {
    RemoveAllLines();
    bounds_ = new_bounds;
    dirty_start_ = text::Offset::Max();
    zoom_ = new_zoom;
    return;
  }

  RemoveDirtyLines();
  if (bounds_ == new_bounds)
    return;

  if (!lines_.empty() && bounds_.width() != new_bounds.width()) {
    std::vector<text::Offset> dirty_offsets;
    for (const auto& it : lines_) {
      const auto& line = it.second;
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
  bounds_ = new_bounds;
}

bool RootInlineBoxCache::IsAfterNewline(const RootInlineBox* text_line) const {
  const auto start = text_line->text_start();
  return !start || buffer_->GetCharAt(start - text::OffsetDelta(1)) == '\n';
}

bool RootInlineBoxCache::IsDirty(const gfx::RectF& bounds, float zoom) const {
  if (zoom_ != zoom)
    return false;
  if (dirty_start_ != text::Offset::Max())
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
  lines_[line->text_start()] = std::move(line_ptr);
#if _DEBUG
  const auto& it = lines_.find(line->text_start());
  if (it != lines_.begin())
    DCHECK_LE(std::prev(it)->second->text_end(), line->text_start());
  if (std::next(it) != lines_.end())
    DCHECK_GE(std::next(it)->second->text_start(), line->text_end());
#endif
  return line;
}

void RootInlineBoxCache::RemoveDirtyLines() {
  const auto dirty_start = dirty_start_;
  dirty_start_ = text::Offset::Max();
  if (lines_.empty() || dirty_start >= lines_.rbegin()->second->text_end())
    return;
  auto it = lines_.lower_bound(dirty_start);
  if (it == lines_.end()) {
    it = lines_.find(lines_.rbegin()->first);
    if (it->second->text_end() < dirty_start)
      return;
  } else {
    DCHECK_GE(it->second->text_start(), dirty_start);
  }
  while (it != lines_.begin() && it->second->text_start() > dirty_start)
    --it;
  DCHECK(it != lines_.end());

  if (it == lines_.begin()) {
    RemoveAllLines();
    return;
  }

  DCHECK_GE(dirty_start, it->second->text_start());
  while (it != lines_.end() && it->second->text_start() < dirty_start &&
         it->second->text_end() < dirty_start) {
    ++it;
  }

  std::vector<text::Offset> dirty_offsets;
  while (it != lines_.end()) {
    dirty_offsets.push_back(it->first);
    ++it;
  }

  for (auto offset : dirty_offsets) {
    const auto present = lines_.find(offset);
    DCHECK(present != lines_.end());
    lines_.erase(present);
  }
}

void RootInlineBoxCache::RemoveAllLines() {
  lines_.clear();
}

}  // namespace layout

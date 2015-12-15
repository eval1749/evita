// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/layout/root_inline_box_cache.h"

#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/layout/root_inline_box.h"
#include "evita/text/buffer.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
RootInlineBoxCache::RootInlineBoxCache(const text::Buffer* buffer)
    : buffer_(buffer), dirty_start_(text::Offset::Max()), zoom_(0.0f) {}

RootInlineBoxCache::~RootInlineBoxCache() {
  for (const auto& entry : lines_)
    entry.second->Release();
}

void RootInlineBoxCache::DidChangeBuffer(text::Offset offset) {
  ASSERT_DOM_LOCKED();
  dirty_start_ = std::min(dirty_start_, offset);
}

RootInlineBox* RootInlineBoxCache::FindLine(text::Offset text_offset) const {
  UI_ASSERT_DOM_LOCKED();
  const auto it = lines_.find(text_offset);
  if (it == lines_.end())
    return nullptr;
  auto const line = it->second;
  DCHECK_EQ(line->text_start(), text_offset);
  return line;
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
    for (auto it : lines_) {
      auto const line = it.second;
      if (line->right() > new_bounds.right || !IsAfterNewline(line) ||
          !IsEndWithNewline(line)) {
        dirty_offsets.push_back(line->text_start());
      }
    }
    for (auto offset : dirty_offsets) {
      const auto it = lines_.find(offset);
      DCHECK(it != lines_.end());
      it->second->Release();
      lines_.erase(it);
    }
  }
  bounds_ = new_bounds;
}

bool RootInlineBoxCache::IsAfterNewline(const RootInlineBox* text_line) const {
  auto const start = text_line->text_start();
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
  auto const end = text_line->text_end();
  return end >= buffer_->GetEnd() || buffer_->GetCharAt(end) == '\n';
}

void RootInlineBoxCache::Register(RootInlineBox* line) {
  UI_ASSERT_DOM_LOCKED();
  DCHECK_GE(line->text_end(), line->text_start());
  lines_[line->text_start()] = line;
#if _DEBUG
  auto it = lines_.find(line->text_start());
  if (it != lines_.begin()) {
    --it;
    DCHECK_LE(it->second->text_end(), line->text_start());
    ++it;
  }
  ++it;
  if (it != lines_.end())
    DCHECK_GE(it->second->text_start(), line->text_end());
#endif
}

void RootInlineBoxCache::RemoveDirtyLines() {
  auto const dirty_start = dirty_start_;
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
    present->second->Release();
    lines_.erase(present);
  }
}

void RootInlineBoxCache::RemoveAllLines() {
  for (const auto& entry : lines_)
    entry.second->Release();
  lines_.clear();
}

}  // namespace layout

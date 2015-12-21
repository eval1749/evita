// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>
#include <vector>

#include "evita/layout/block_flow.h"

#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/buffer.h"
#include "evita/text/static_range.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/line/root_inline_box_cache.h"
#include "evita/layout/render_style.h"
#include "evita/layout/text_formatter.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// BlockFlow
//
BlockFlow::BlockFlow(text::Buffer* text_buffer)
    : text_buffer_(text_buffer),
      text_line_cache_(new RootInlineBoxCache(*text_buffer)) {}

BlockFlow::~BlockFlow() {}

text::Offset BlockFlow::text_end() const {
  UI_ASSERT_DOM_LOCKED();
  return lines_.back()->text_end();
}

text::Offset BlockFlow::text_start() const {
  UI_ASSERT_DOM_LOCKED();
  return lines_.front()->text_start();
}

void BlockFlow::Append(RootInlineBox* line) {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty()) {
    DCHECK_EQ(lines_height_, 0.0f);
    line->set_origin(gfx::PointF());
  } else {
    DCHECK(lines_.back()->text_end() == line->text_start());
    if (!dirty_line_point_) {
      const auto& last_line = lines_.back();
      line->set_origin(gfx::PointF(last_line->left(), last_line->bottom()));
    }
  }
  lines_height_ += line->height();
  lines_.push_back(std::move(line));
}

text::Offset BlockFlow::ComputeEndOfLine(text::Offset text_offset) {
  TRACE_EVENT0("views", "BlockFlow::ComputeEndOfLine");
  UI_ASSERT_DOM_LOCKED();

  if (text_offset >= text_buffer_->GetEnd())
    return text_buffer_->GetEnd();

  EnsureTextLineCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->text_end() - text::OffsetDelta(1);

  const auto line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(*text_buffer_, line_start, line_start, bounds_,
                          zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (text_offset < line->text_end())
      return line->text_end() - text::OffsetDelta(1);
  }
}

text::Offset BlockFlow::ComputeStartOfLine(text::Offset text_offset) {
  TRACE_EVENT0("views", "BlockFlow::ComputeStartOfLine");
  UI_ASSERT_DOM_LOCKED();
  DCHECK(text_offset.IsValid());

  if (text_offset == text::Offset(0))
    return text::Offset(0);

  EnsureTextLineCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->text_start();

  const auto line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(*text_buffer_, line_start, line_start, bounds_,
                          zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (text_offset < line->text_end())
      return line->text_start();
  }
}

text::Offset BlockFlow::ComputeVisibleEnd() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  for (auto it = lines_.crbegin(); it != lines_.crend(); ++it) {
    auto const line = *it;
    if (line->bounds().bottom <= bounds_.bottom)
      return line->text_end();
  }
  return lines_.front()->text_end();
}

void BlockFlow::DidChangeStyle(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  MarkDirty();
}

void BlockFlow::DidDeleteAt(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  MarkDirty();
  if (view_start_ <= range.start())
    return;
  view_start_ = std::max(view_start_ - range.length(), range.start());
}

void BlockFlow::DidInsertBefore(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  MarkDirty();
  if (view_start_ <= range.start())
    return;
  view_start_ = view_start_ + range.length();
}

bool BlockFlow::DiscardFirstLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  const auto& line = lines_.front();
  lines_height_ -= line->height();
  lines_.pop_front();
  dirty_line_point_ = true;
  return true;
}

bool BlockFlow::DiscardLastLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  const auto& line = lines_.back();
  lines_height_ -= line->height();
  lines_.pop_back();
  return true;
}

void BlockFlow::EnsureLinePoints() {
  UI_ASSERT_DOM_LOCKED();
  if (!dirty_line_point_)
    return;
  auto line_top = 0.0f;
  for (const auto& line : lines_) {
    line->set_origin(gfx::PointF(0.0f, line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void BlockFlow::EnsureTextLineCache() {
  UI_ASSERT_DOM_LOCKED();
  text_line_cache_->Invalidate(gfx::RectF(bounds_.size()), zoom_);
}

RootInlineBox* BlockFlow::FindLineContainng(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::HitTestTextPosition");
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  if (offset < lines_.front()->text_start())
    return nullptr;
  if (offset >= lines_.back()->text_end())
    return nullptr;
  // Get line after |offset|.
  const auto& it =
      std::lower_bound(lines_.begin(), lines_.end(), offset,
                       [](const RootInlineBox* box, text::Offset value) {
                         return box->text_start() < value;
                       });
  if (it == lines_.begin())
    return lines_.front();
  if (it == lines_.end())
    return lines_.back();
  if ((*it)->text_start() == offset)
    return *it;
  return *std::prev(it);
}

void BlockFlow::Format(text::Offset text_offset) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "BlockFlow::Format");
  EnsureTextLineCache();
  lines_.clear();
  lines_height_ = 0;
  dirty_line_point_ = false;

  const auto line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(*text_buffer_, line_start, line_start, bounds_,
                          zoom_);
  for (;;) {
    const auto line = FormatLine(&formatter);
    DCHECK_GT(line->bounds().height(), 0.0f);
    Append(line);

    // Line must have at least one cell other than filler.
    DCHECK_GE(line->text_end(), line->text_start());

    if (lines_height_ >= bounds_.height()) {
      // BlockFlow is filled up with lines.
      break;
    }

    if (line->text_end() > text_buffer_->GetEnd()) {
      // We have no more contents.
      break;
    }
  }

  // Scroll up until we have |text_offset| in this |BlockFlow|.
  while (lines_.back()->text_end() < text_buffer_->GetEnd() &&
         text_offset > lines_.front()->text_end()) {
    DiscardFirstLine();
    Append(FormatLine(&formatter));
  }
  EnsureLinePoints();
  view_start_ = lines_.front()->text_start();
  ++version_;
}

bool BlockFlow::FormatIfNeeded() {
  UI_ASSERT_DOM_LOCKED();
  EnsureTextLineCache();
  if (!NeedsFormat())
    return false;
  Format(view_start_);
  return true;
}

RootInlineBox* BlockFlow::FormatLine(TextFormatter* formatter) {
  UI_ASSERT_DOM_LOCKED();
  const auto& cached_line =
      text_line_cache_->FindLine(formatter->text_offset());
  if (cached_line) {
    formatter->DidFormat(cached_line);
    return cached_line;
  }
  return text_line_cache_->Register(std::move(formatter->FormatLine()));
}

text::Offset BlockFlow::HitTestPoint(gfx::PointF block_point) const {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::HitTestPoint");
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  // TODO(eval1749): We should transform |block_point| to content point for
  // considering margin, border and padding.
  const auto& content_point = block_point;
  const auto& it =
      std::lower_bound(lines_.begin(), lines_.end(), content_point.y,
                       [](const RootInlineBox* box, float point_y) {
                         return box->top() < point_y;
                       });
  if (it == lines_.begin())
    return lines_.front()->HitTestPoint(content_point.x);
  if (it == lines_.end())
    return lines_.back()->HitTestPoint(content_point.x);
  if ((*it)->top() == content_point.y)
    return (*it)->HitTestPoint(content_point.x);
  return (*std::prev(it))->HitTestPoint(content_point.x);
}

gfx::RectF BlockFlow::HitTestTextPosition(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::HitTestTextPosition");
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  const auto line = FindLineContainng(offset);
  if (!line)
    return gfx::RectF();
  const auto& rect = line->HitTestTextPosition(offset);
  if (rect.empty())
    return gfx::RectF();
  return gfx::RectF(rect.origin() + bounds_.origin(), rect.size());
}

bool BlockFlow::IsFullyVisibleTextPosition(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  return offset >= text_start() && offset < ComputeVisibleEnd();
}

bool BlockFlow::IsShowEndOfDocument() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!ShouldFormat());
  DCHECK(!dirty_line_point_);
  auto const last_line = lines_.back();
  return last_line->IsEndOfDocument() && last_line->bottom() <= bounds_.bottom;
}

text::Offset BlockFlow::MapPointXToOffset(text::Offset text_offset,
                                          float point_x) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::MapPointXToOffset");
  EnsureTextLineCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->HitTestPoint(point_x);

  const auto line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(*text_buffer_, line_start, line_start, bounds_,
                          zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (text_offset < line->text_end())
      return line->HitTestPoint(point_x);
  }
}

void BlockFlow::MarkDirty() {
  lines_.clear();
  dirty_line_point_ = true;
  lines_height_ = 0;
}

bool BlockFlow::NeedsFormat() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!text_line_cache_->IsDirty(gfx::RectF(bounds_.size()), zoom_));
  if (lines_.empty())
    return true;
  for (const auto& line : lines_) {
    if (line != text_line_cache_->FindLine(line->text_start()))
      return true;
  }
  return false;
}

void BlockFlow::Prepend(RootInlineBox* line) {
  UI_ASSERT_DOM_LOCKED();
  lines_height_ += line->height();
  lines_.push_front(std::move(line));
  dirty_line_point_ = true;
}

bool BlockFlow::ScrollDown() {
  TRACE_EVENT0("views", "BlockFlow::ScrollDown");
  FormatIfNeeded();
  if (!lines_.front()->text_start())
    return false;
  ++version_;
  const auto goal_offset = lines_.front()->text_start() - text::OffsetDelta(1);
  const auto line_start = text_buffer_->ComputeStartOfLine(goal_offset);
  TextFormatter formatter(*text_buffer_, line_start, line_start, bounds_,
                          zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (goal_offset < line->text_end()) {
      Prepend(std::move(line));
      break;
    }
  }

  // Discard lines outside of screen.
  EnsureLinePoints();
  while (lines_.back()->top() >= bounds_.bottom) {
    DiscardLastLine();
  }
  view_start_ = lines_.front()->text_start();
  return true;
}

bool BlockFlow::ScrollToPosition(text::Offset offset) {
  TRACE_EVENT0("views", "BlockFlow::ScrollToPosition");
  FormatIfNeeded();

  if (IsFullyVisibleTextPosition(offset))
    return false;

  const auto scrollable = bounds_.height() / 2;

  if (offset > text_start()) {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      const auto scroll_height = lines_.front()->height();
      if (!ScrollUp())
        return scrolled > 0.0f;
      if (IsFullyVisibleTextPosition(offset))
        return true;
      scrolled += scroll_height;
    }
  } else {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      auto const scroll_height = lines_.back()->height();
      if (!ScrollDown())
        return scrolled > 0.0f;
      if (IsFullyVisibleTextPosition(offset))
        return true;
      scrolled += scroll_height;
    }
  }

  Format(offset);
  // When start of line is very far from |offset|, |offset| is below view port.
  // We scroll up until |offset| is in view port.
  while (!IsFullyVisibleTextPosition(offset)) {
    if (!ScrollUp())
      return true;
  }

  // If this page shows end of buffer, we shows lines as much as
  // possible to fit in page.
  auto const buffer_end = text_buffer_->GetEnd();
  if (text_end() >= buffer_end) {
    while (IsFullyVisibleTextPosition(buffer_end)) {
      if (!ScrollDown())
        return true;
    }
    ScrollUp();
    return true;
  }

  // Move line containing |offset| to middle of screen.
  auto scrolled = HitTestTextPosition(offset).top - bounds_.top;
  if (scrolled < scrollable) {
    while (scrolled < scrollable) {
      auto const scroll_height = lines_.front()->height();
      if (!ScrollDown())
        return true;
      scrolled += scroll_height;
    }
    return true;
  }

  scrolled = scrollable + scrolled;
  while (scrolled < scrollable) {
    auto const scroll_height = lines_.back()->height();
    if (!ScrollUp())
      return true;
    scrolled += scroll_height;
  }
  return true;
}

bool BlockFlow::ScrollUp() {
  TRACE_EVENT0("views", "BlockFlow::ScrollUp");
  FormatIfNeeded();
  EnsureLinePoints();
  if (IsShowEndOfDocument())
    return false;

  ++version_;

  for (;;) {
    if (!DiscardFirstLine())
      return false;
    EnsureLinePoints();

    view_start_ = lines_.front()->text_start();
    if (!lines_.back()->IsEndOfDocument())
      break;
    if (IsShowEndOfDocument())
      return true;
  }

  const auto last_line = lines_.back();
  const auto offset = last_line->text_end();
  const auto line_start =
      last_line->IsContinuedLine() ? last_line->line_start() : offset;
  TextFormatter formatter(*text_buffer_, line_start, offset, bounds_, zoom_);
  Append(FormatLine(&formatter));
  return true;
}

void BlockFlow::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  MarkDirty();
}

void BlockFlow::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  MarkDirty();
}

bool BlockFlow::ShouldFormat() const {
  UI_ASSERT_DOM_LOCKED();
  return lines_.empty();
}

}  // namespace layout

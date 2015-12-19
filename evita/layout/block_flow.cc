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
    : dirty_(true),
      dirty_line_point_(true),
      format_counter_(0),
      lines_height_(0),
      need_format_(false),
      text_buffer_(text_buffer),
      text_line_cache_(new RootInlineBoxCache(text_buffer)),
      view_start_(0),
      zoom_(1.0f) {}

BlockFlow::~BlockFlow() {}

void BlockFlow::Append(scoped_refptr<RootInlineBox> line) {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty()) {
    DCHECK_EQ(lines_height_, 0.0f);
    line->set_origin(bounds_.origin());
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

void BlockFlow::DidChangeStyle(text::Offset offset, text::OffsetDelta) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
}

void BlockFlow::DidDeleteAt(text::Offset offset, text::OffsetDelta length) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(view_start_ - length, offset);
}

void BlockFlow::DidInsertBefore(text::Offset offset, text::OffsetDelta length) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
  if (view_start_ <= offset)
    return;
  view_start_ = view_start_ + length;
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

text::Offset BlockFlow::EndOfLine(text::Offset text_offset) {
  TRACE_EVENT0("views", "BlockFlow::EndOfLine");
  UI_ASSERT_DOM_LOCKED();

  if (text_offset >= text_buffer_->GetEnd())
    return text_buffer_->GetEnd();

  InvalidateCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->text_end() - text::OffsetDelta(1);

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (text_offset < line->text_end())
      return line->text_end() - text::OffsetDelta(1);
  }
}

void BlockFlow::EnsureLinePoints() {
  UI_ASSERT_DOM_LOCKED();
  if (!dirty_line_point_)
    return;
  auto line_top = bounds_.top;
  for (const auto& line : lines_) {
    line->set_origin(gfx::PointF(bounds_.left, line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void BlockFlow::Format(text::Offset text_offset) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "BlockFlow::Format");
  InvalidateCache();
  lines_.clear();
  lines_height_ = 0;
  dirty_ = false;
  dirty_line_point_ = false;
  need_format_ = false;

  auto const line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, line_start, bounds_, zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
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
  ++format_counter_;
}

bool BlockFlow::FormatIfNeeded() {
  UI_ASSERT_DOM_LOCKED();
  text_line_cache_->Invalidate(bounds_, zoom_);
  if (!NeedFormat()) {
    dirty_ = false;
    return false;
  }
  Format(view_start_);
  return true;
}

scoped_refptr<RootInlineBox> BlockFlow::FormatLine(TextFormatter* formatter) {
  UI_ASSERT_DOM_LOCKED();
  const auto& cached_line =
      text_line_cache_->FindLine(formatter->text_offset());
  if (cached_line) {
    formatter->set_text_offset(cached_line->text_end());
    return cached_line;
  }
  const auto& line = formatter->FormatLine();
  text_line_cache_->Register(line.get());
  return line;
}

text::Offset BlockFlow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  return lines_.back()->text_end();
}

text::Offset BlockFlow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  return lines_.front()->text_start();
}

text::Offset BlockFlow::GetVisibleEnd() {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "BlockFlow::GetVisibleEnd");
  FormatIfNeeded();
  DCHECK(!dirty_line_point_);
  for (auto it = lines_.crbegin(); it != lines_.crend(); ++it) {
    auto const line = *it;
    if (line->bounds().bottom <= bounds_.bottom)
      return line->text_end();
  }
  return lines_.front()->text_end();
}

text::Offset BlockFlow::HitTestPoint(gfx::PointF point) const {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::HitTestPoint");
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  const auto& it =
      std::lower_bound(lines_.begin(), lines_.end(), point.y,
                       [](const scoped_refptr<RootInlineBox>& box,
                          float point_y) { return box->top() < point_y; });
  if (it == lines_.begin())
    return lines_.front()->text_start();
  if (it == lines_.end())
    return lines_.back()->text_end();
  const auto line = *std::prev(it);
  return line->MapXToPosn(point.x);
}

gfx::RectF BlockFlow::HitTestTextPosition(text::Offset offset) const {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::HitTestTextPosition");
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  if (offset < lines_.front()->text_start() ||
      offset > lines_.back()->text_end()) {
    return gfx::RectF();
  }
  for (const auto& line : lines_) {
    auto const rect = line->HitTestTextPosition(offset);
    if (!rect.empty())
      return rect;
  }
  return gfx::RectF();
}

void BlockFlow::InvalidateCache() {
  UI_ASSERT_DOM_LOCKED();
  text_line_cache_->Invalidate(bounds_, zoom_);
  if (!dirty_)
    return;
  lines_.clear();
  lines_height_ = 0;
}

void BlockFlow::InvalidateLines(text::Offset offset) {
  ASSERT_DOM_LOCKED();
  text_line_cache_->DidChangeBuffer(offset);
  dirty_ = true;
}

bool BlockFlow::IsPositionFullyVisible(text::Offset offset) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "BlockFlow::IsPositionFullyVisible");
  FormatIfNeeded();
  return offset >= GetStart() && offset < GetVisibleEnd();
}

bool BlockFlow::IsShowEndOfDocument() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  auto const last_line = lines_.back();
  return last_line->IsEndOfDocument() && last_line->bottom() <= bounds_.bottom;
}

text::Offset BlockFlow::MapPointXToOffset(text::Offset text_offset,
                                          float point_x) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("views", "BlockFlow::MapPointXToOffset");
  InvalidateCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->MapXToPosn(point_x);

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    if (text_offset < line->text_end())
      return line->MapXToPosn(point_x);
  }
}

bool BlockFlow::NeedFormat() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!text_line_cache_->IsDirty(bounds_, zoom_));
  if (need_format_)
    return true;
  if (!dirty_)
    return false;
  if (lines_.empty())
    return true;
  for (const auto& line : lines_) {
    if (line != text_line_cache_->FindLine(line->text_start()))
      return true;
  }
  return false;
}

void BlockFlow::Prepend(scoped_refptr<RootInlineBox> line) {
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
  ++format_counter_;
  auto const goal_offset = lines_.front()->text_start() - text::OffsetDelta(1);
  auto const start_offset = text_buffer_->ComputeStartOfLine(goal_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
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

  if (IsPositionFullyVisible(offset))
    return false;

  const auto scrollable = bounds_.height() / 2;

  if (offset > GetStart()) {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      const auto scroll_height = lines_.front()->height();
      if (!ScrollUp())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  } else {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      auto const scroll_height = lines_.back()->height();
      if (!ScrollDown())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  }

  Format(offset);
  // When start of line is very far from |offset|, |offset| is below view port.
  // We scroll up until |offset| is in view port.
  while (!IsPositionFullyVisible(offset)) {
    if (!ScrollUp())
      return true;
  }

  // If this page shows end of buffer, we shows lines as much as
  // possible to fit in page.
  auto const buffer_end = text_buffer_->GetEnd();
  if (GetEnd() >= buffer_end) {
    while (IsPositionFullyVisible(buffer_end)) {
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

  ++format_counter_;

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

  auto const start_offset = lines_.back()->text_end();
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  Append(FormatLine(&formatter));
  return true;
}

void BlockFlow::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  dirty_ = true;
  need_format_ = true;
}

void BlockFlow::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  dirty_ = true;
  need_format_ = true;
}

bool BlockFlow::ShouldFormat() const {
  UI_ASSERT_DOM_LOCKED();
  return dirty_;
}

text::Offset BlockFlow::StartOfLine(text::Offset text_offset) {
  TRACE_EVENT0("views", "BlockFlow::StartOfLine");
  UI_ASSERT_DOM_LOCKED();
  DCHECK(text_offset.IsValid());

  if (text_offset == text::Offset(0))
    return text::Offset(0);

  InvalidateCache();
  if (const auto& line = text_line_cache_->FindLine(text_offset))
    return line->text_start();

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    const auto& line = FormatLine(&formatter);
    start_offset = line->text_end();
    if (text_offset < start_offset)
      return line->text_start();
  }
}

}  // namespace layout

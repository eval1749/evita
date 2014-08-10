// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"

namespace views {
namespace rendering {

TextBlock::TextBlock(text::Buffer* text_buffer, const gfx::RectF& bounds,
                     float zoom)
    : bounds_(bounds), dirty_(true), dirty_line_point_(true), lines_height_(0),
      new_zoom_(1.0f), text_buffer_(text_buffer), zoom_(zoom) {
  text_buffer->AddObserver(this);
}

TextBlock::TextBlock(text::Buffer* text_buffer)
    : TextBlock(text_buffer, gfx::RectF(), 0.0f) {
}

TextBlock::~TextBlock() {
  text_buffer_->RemoveObserver(this);
}

void TextBlock::Append(TextLine* line) {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(lines_.empty() || lines_.back()->GetEnd() == line->GetStart());
  if (!dirty_line_point_) {
    auto const last_line = lines_.back();
    line->set_origin(gfx::PointF(last_line->left(), last_line->bottom()));
  }
  lines_.push_back(line);
  lines_height_ += line->GetHeight();
}

bool TextBlock::DiscardFirstLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.front();
  lines_height_ -= line->GetHeight();
  delete line;
  lines_.pop_front();
  dirty_line_point_ = true;
  return true;
}

bool TextBlock::DiscardLastLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.back();
  lines_height_ -= line->GetHeight();
  delete line;
  lines_.pop_back();
  return true;
}

text::Posn TextBlock::EndOfLine(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset >= text_buffer_->GetEnd())
    return text_buffer_->GetEnd();

  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->GetEnd() - 1;
  }

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->GetEnd() - 1;
  }
}

void TextBlock::EnsureLinePoints() {
  UI_ASSERT_DOM_LOCKED();
  if (!dirty_line_point_)
    return;
  auto line_top = top();
  for (auto line : lines_) {
    line->set_origin(gfx::PointF(left(), line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

TextLine* TextBlock::FindLine(Posn text_offset) const {
  DCHECK(!ShouldFormat());
  if (text_offset < GetFirst()->GetStart() ||
      text_offset > GetFirst()->GetEnd()) {
    return nullptr;
  }
  for (auto const line : lines_) {
    if (text_offset < line->text_end())
      return const_cast<TextLine*>(line);
  }
  return nullptr;
}

void TextBlock::Format(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  bounds_ = new_bounds_;
  zoom_ = new_zoom_;
  Reset();

  // TODO(eval1749) We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& style = text_buffer_->GetDefaultStyle();
  // |default_style_| is used for providing fill color of right of end of line
  // and bottom of text block.
  default_style_ = RenderStyle(style, nullptr);

  auto const line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, line_start, new_bounds_, new_zoom_);
  for (;;) {
    auto const line = formatter.FormatLine();
    DCHECK_GT(line->bounds().height(), 0.0f);
    Append(line);

    // Line must have at least one cell other than filler.
    DCHECK_GE(line->GetEnd(), line->GetStart());

    if (lines_height_ >= bounds_.height()) {
      // TextBlock is filled up with lines.
      break;
    }

    if (line->GetEnd() > text_buffer_->GetEnd()) {
      // We have no more contents.
      break;
    }
  }

  dirty_ = false;
  dirty_line_point_ = true;

  // Scroll up until we have |text_offset| in this |TextBlock|.
  while (text_offset > GetFirst()->GetEnd()) {
    DiscardLastLine();
    Append(formatter.FormatLine());
  }
  EnsureLinePoints();
}

text::Posn TextBlock::GetVisibleEnd() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  for (auto it = lines_.crbegin(); it != lines_.crend(); ++it) {
    auto const line = *it;
    if (line->bounds().bottom <= bounds_.bottom)
      return line->GetEnd();
  }
  return lines_.front()->GetEnd();
}

gfx::RectF TextBlock::HitTestTextPosition(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();
  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->HitTestTextPosition(text_offset);
  }

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->HitTestTextPosition(text_offset);
  }
}

void TextBlock::InvalidateLines(text::Posn offset) {
  ASSERT_DOM_LOCKED();
  if (dirty_)
    return;
  if (GetFirst()->GetStart() >= offset || GetLast()->GetEnd() >= offset)
    Reset();
}

text::Posn TextBlock::MapPointXToOffset(text::Posn text_offset,
                                        float point_x) const {
  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->MapXToPosn(point_x);
  }

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->MapXToPosn(point_x);
  }
}

bool TextBlock::IsShowEndOfDocument() const {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  return GetLast()->GetEnd() > text_buffer_->GetEnd() &&
         GetLast()->bottom() <= bounds_.bottom;
}

void TextBlock::Prepend(TextLine* line) {
  UI_ASSERT_DOM_LOCKED();
  lines_.push_front(line);
  lines_height_ += line->GetHeight();
  dirty_line_point_ = true;
}

void TextBlock::Reset() {
  ASSERT_DOM_LOCKED();
  for (auto const line : lines_) {
    delete line;
  }

  lines_.clear();
  dirty_ = true;
  dirty_line_point_ = true;
  lines_height_ = 0;
}

bool TextBlock::ScrollDown() {
  DCHECK(!ShouldFormat());
  if (!GetFirst()->GetStart())
    return false;
  auto const goal_offset = GetFirst()->GetStart() - 1;
  auto const start_offset = text_buffer_->ComputeStartOfLine(goal_offset);
  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  for (;;) {
    auto const line = formatter.FormatLine();
    if (goal_offset < line->GetEnd()) {
      Prepend(line);
      break;
    }
  }

  // Discard lines outside of screen.
  EnsureLinePoints();
  while (GetLast()->top() >= bounds_.bottom) {
    DiscardLastLine();
  }
  return true;
}

bool TextBlock::ScrollUp() {
  DCHECK(!ShouldFormat());
  EnsureLinePoints();
  if (IsShowEndOfDocument())
    return false;

  if (!DiscardFirstLine())
    return false;

  EnsureLinePoints();
  if (IsShowEndOfDocument())
    return true;

  auto const start_offset = GetLast()->GetEnd();
  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  auto const line = formatter.FormatLine();
  Append(line);
  return true;
}

void TextBlock::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  new_bounds_ = new_bounds;
}

void TextBlock::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  new_zoom_ = new_zoom;
}

bool TextBlock::ShouldFormat() const {
  UI_ASSERT_DOM_LOCKED();
  // TODO(eval1749) We should check bounds change more. We don't need to
  // format when
  //  - Height changes only
  //  - Narrow but all lines fit
  //  - Widen but no lines wrap
  return dirty_ || zoom_ != new_zoom_ || bounds_ != new_bounds_;
}

text::Posn TextBlock::StartOfLine(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset <= 0)
    return 0;

  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->GetStart();
  }

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  if (!start_offset)
    return 0;

  TextFormatter formatter(text_buffer_, start_offset, new_bounds_, new_zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    start_offset = line->GetEnd();
    if (text_offset < start_offset)
      return line->GetStart();
  }
}

// text::BufferMutationObserver
void TextBlock::DidChangeStyle(Posn offset, size_t) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
}

void TextBlock::DidDeleteAt(Posn offset, size_t) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
}

void TextBlock::DidInsertAt(Posn offset, size_t) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
}

}  // namespace rendering
}  // namespace views

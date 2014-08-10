// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include "evita/dom/lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"

namespace views {
namespace rendering {

TextBlock::TextBlock(text::Buffer* text_buffer, const gfx::RectF& bounds,
                     float zoom)
    : bounds_(bounds), dirty_(true), dirty_line_point_(true),
      lines_height_(0), text_buffer_(text_buffer), zoom_(zoom) {
  text_buffer->AddObserver(this);
}

TextBlock::TextBlock(text::Buffer* text_buffer)
    : TextBlock(text_buffer, gfx::RectF(), 0.0f) {
}

TextBlock::~TextBlock() {
  text_buffer_->RemoveObserver(this);
}

void TextBlock::Append(TextLine* line) {
  ASSERT_DOM_LOCKED();
  DCHECK(lines_.empty() || lines_.back()->GetEnd() == line->GetStart());
  if (!dirty_line_point_) {
    auto const last_line = lines_.back();
    line->set_origin(gfx::PointF(last_line->left(), last_line->bottom()));
  }
  lines_.push_back(line);
  lines_height_ += line->GetHeight();
}

bool TextBlock::DiscardFirstLine() {
  ASSERT_DOM_LOCKED();
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
  ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.back();
  lines_height_ -= line->GetHeight();
  delete line;
  lines_.pop_back();
  return true;
}

void TextBlock::EnsureLinePoints() {
  ASSERT_DOM_LOCKED();
  if (!dirty_line_point_)
    return;
  auto line_top = top();
  for (auto line : lines_) {
    line->set_origin(gfx::PointF(left(), line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void TextBlock::Format(const gfx::RectF& bounds, float zoom,
                       text::Posn text_offset) {
  DCHECK(!bounds.empty());
  DCHECK_GT(zoom, 0.0f);
  ASSERT_DOM_LOCKED();
  bounds_ = bounds;
  zoom_ = zoom;
  Reset();

  // TODO(eval1749) We should recompute default style when style is chagned,
  // rather than every |Format| call.
  const auto& style = text_buffer_->GetDefaultStyle();
  default_style_ = RenderStyle(style, nullptr);

  TextFormatter formatter(this, text_buffer_->ComputeStartOfLine(text_offset));
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
  dirty_line_point_ = false;

  // Scroll up until we have |text_offset| in this |TextBlock|.
  while (text_offset > GetFirst()->GetEnd()) {
    DiscardLastLine();
    Append(formatter.FormatLine());
  }
  EnsureLinePoints();
}

text::Posn TextBlock::GetVisibleEnd() const {
  ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  for (auto it = lines_.crbegin(); it != lines_.crend(); ++it) {
    auto const line = *it;
    if (line->bounds().bottom <= bounds_.bottom)
      return line->GetEnd();
  }
  return lines_.front()->GetEnd();
}

void TextBlock::InvalidateLines(text::Posn offset) {
  ASSERT_DOM_LOCKED();
  if (dirty_)
    return;
  if (GetFirst()->GetStart() >= offset || GetLast()->GetEnd() >= offset)
    Reset();
}

bool TextBlock::IsShowEndOfDocument() const {
  ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  return GetLast()->GetEnd() > text_buffer_->GetEnd() &&
         GetLast()->bottom() <= bounds_.bottom;
}

void TextBlock::Prepend(TextLine* line) {
  ASSERT_DOM_LOCKED();
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

bool TextBlock::ShouldFormat(const gfx::RectF& bounds, float zoom) const {
  ASSERT_DOM_LOCKED();
  // TODO(eval1749) We should check bounds change more. We don't need to
  // format when
  //  - Height changes only
  //  - Narrow but all lines fit
  //  - Widen but no lines wrap
  return dirty_ || zoom_ != zoom || bounds_ != bounds;
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

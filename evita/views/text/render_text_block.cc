// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include "evita/dom/lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"

namespace views {
namespace rendering {

TextBlock::TextBlock(text::Buffer* text_buffer)
    : dirty_(true),
      dirty_line_point_(true),
      m_cy(0), text_buffer_(text_buffer), zoom_(0.0f) {
  text_buffer->AddObserver(this);
}

TextBlock::~TextBlock() {
  text_buffer_->RemoveObserver(this);
}

void TextBlock::Append(TextLine* pLine) {
  ASSERT_DOM_LOCKED();
  if (!dirty_line_point_) {
    auto const last_line = lines_.back();
    pLine->set_origin(gfx::PointF(last_line->left(), last_line->bottom()));
  }
  lines_.push_back(pLine);
  m_cy += pLine->GetHeight();
}

bool TextBlock::DiscardFirstLine() {
  ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.front();
  m_cy -= line->GetHeight();
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
  m_cy -= line->GetHeight();
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

void TextBlock::Finish() {
  ASSERT_DOM_LOCKED();
  dirty_ = lines_.empty();
  dirty_line_point_ = dirty_;
}

void TextBlock::Format(const gfx::RectF& bounds, float zoom,
                       text::Posn text_offset) {
  ASSERT_DOM_LOCKED();
  if (dirty_ || bounds_ != bounds || zoom_ != zoom) {
    bounds_ = bounds;
    zoom_ = zoom;
    Reset();
  }
  TextFormatter formatter(this, text_offset);
  formatter.Format();
}

TextLine* TextBlock::FormatLine(const gfx::RectF& bounds, float zoom,
                                text::Posn text_offset) {
  ASSERT_DOM_LOCKED();
  if (dirty_ || bounds_ != bounds || zoom_ != zoom) {
    bounds_ = bounds;
    zoom_ = zoom;
    Reset();
  }
  for (auto line : lines_) {
    if (line->GetStart() == text_offset)
      return line;
  }
  TextFormatter formatter(this, text_offset);
  return formatter.FormatLine();
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
  m_cy += line->GetHeight();
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
  m_cy = 0;
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

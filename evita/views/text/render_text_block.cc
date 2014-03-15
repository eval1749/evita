// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include "evita/dom/lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_text_line.h"

namespace views {
namespace rendering {

TextBlock::TextBlock(text::Buffer* text_buffer)
    : dirty_(true),
      dirty_line_point_(true),
      m_cy(0),
      text_buffer_(text_buffer) {
  text_buffer->AddObserver(this);
}

TextBlock::~TextBlock() {
  text_buffer_->RemoveObserver(this);
}

void TextBlock::Append(TextLine* pLine) {
  DCHECK_LT(pLine->GetHeight(), 100.0f);
  if (!dirty_line_point_) {
    auto const last_line = lines_.back();
    pLine->set_left_top(gfx::PointF(last_line->left(), last_line->bottom()));
  }
  lines_.push_back(pLine);
  m_cy += pLine->GetHeight();
}

bool TextBlock::DiscardFirstLine() {
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
  if (lines_.empty())
    return false;

  auto const line = lines_.back();
  m_cy -= line->GetHeight();
  delete line;
  lines_.pop_back();
  return true;
}

void TextBlock::EnsureLinePoints() {
  if (!dirty_line_point_)
    return;
  auto line_top = top();
  for (auto line : lines_) {
    line->set_left_top(gfx::PointF(left(), line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void TextBlock::Finish() {
  dirty_ = lines_.empty();
  dirty_line_point_ = dirty_;
}

void TextBlock::InvalidateLines(text::Posn offset) {
  if (dirty_)
    return;
  if (GetFirst()->GetStart() >= offset || GetLast()->GetEnd() >= offset)
    Reset();
}

bool TextBlock::IsShowEndOfDocument() const {
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  return GetLast()->GetEnd() > text_buffer_->GetEnd() &&
         GetLast()->bottom() <= rect_.bottom;
}

void TextBlock::Prepend(TextLine* line) {
  DCHECK_LT(line->GetHeight(), 100.0f);
  lines_.push_front(line);
  m_cy += line->GetHeight();
  dirty_line_point_ = true;
}

void TextBlock::Reset() {
  for (auto const line : lines_) {
    delete line;
  }

  lines_.clear();
  dirty_ = true;
  dirty_line_point_ = true;
  m_cy = 0;
}

void TextBlock::SetRect(const gfx::RectF& rect) {
  rect_ = rect;
  Reset();
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

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include "evita/views/text/render_text_line.h"

namespace views {
namespace rendering {

TextBlock::TextBlock()
    : dirty_(true),
      dirty_line_point_(true),
      m_cy(0) {
}

TextBlock::~TextBlock() {
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

TextLine* TextBlock::ScrollDown() {
  if (lines_.empty())
    return nullptr;

  auto const line = lines_.back();
  lines_.pop_back();
  m_cy -= line->GetHeight();
  return line;
}

TextLine* TextBlock::ScrollUp() {
  if (lines_.empty())
    return nullptr;

  auto const line = lines_.front();
  lines_.pop_front();
  m_cy -= line->GetHeight();
  dirty_line_point_ = true;
  return line;
}

void TextBlock::SetBufferDirtyOffset(Posn offset) {
  if (dirty_)
    return;
  dirty_ = GetFirst()->GetStart() >= offset ||
           GetLast()->GetEnd() >= offset;
}

void TextBlock::SetRect(const gfx::RectF& rect) {
  rect_ = rect;
  Reset();
}

}  // namespace rendering
}  // namespace views

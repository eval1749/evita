// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_line.h"

#include "evita/views/text/render_cell.h"

namespace views {
namespace rendering {

TextLine::TextLine(const TextLine& other)
    : m_nHash(other.m_nHash),
      m_lEnd(other.m_lEnd),
      m_lStart(other.m_lStart),
      rect_(other.rect_) {
  for (auto cell : other.cells_) {
    cells_.push_back(cell->Copy());
  }
}

TextLine::TextLine()
    : m_nHash(0),
      m_lStart(0),
      m_lEnd(0) {
}

TextLine::~TextLine() {
}

void TextLine::set_left_top(const gfx::PointF& left_top) {
  rect_.right = rect_.width() + left_top.x;
  rect_.bottom = rect_.height() + left_top.y;
  rect_.left = left_top.x;
  rect_.top = left_top.y;
}

TextLine* TextLine::Copy() const {
  return new TextLine(*this);
}

bool TextLine::Equal(const TextLine* other) const {
  if (Hash() != other->Hash())
    return false;
  if (cells_.size() != other->cells_.size())
    return false;
  auto other_it = other->cells_.begin();
  for (auto cell : cells_) {
    if (!cell->Equal(*other_it))
      return false;
    ++other_it;
  }
  return true;
}

void TextLine::AddCell(Cell* cell) {
  cells_.push_back(cell);
}

void TextLine::Fix(float left, float top, float ascent, float descent) {
  auto const height = ascent + descent;
  DCHECK_LT(height, 100.0f);
  auto right = left;
  for (auto cell : cells_) {
    auto const lEnd = cell->Fix(height, descent);
    if (lEnd >= 0)
      m_lEnd = lEnd;
    right += cell->width();
  }
  rect_.left = left;
  rect_.top = top;
  rect_.right = right;
  rect_.bottom = top + height;
}

uint TextLine::Hash() const {
  if (m_nHash)
    return m_nHash;
  for (const auto cell : cells_) {
    m_nHash <<= 5;
    m_nHash ^= cell->Hash();
    m_nHash >>= 3;
  }
  return m_nHash;
}

Posn TextLine::MapXToPosn(const gfx::Graphics& gfx, float xGoal) const {
  auto xCell = 0.0f;
  auto lPosn = GetEnd() - 1;
  for (const auto cell : cells_) {
    auto const x = xGoal - xCell;
    xCell += cell->width();
    auto const lMap = cell->MapXToPosn(gfx, x);
    if (lMap >= 0)
      lPosn = lMap;
    if (x >= 0 && x < cell->width())
      break;
  }
  return lPosn;
}

void TextLine::Render(const gfx::Graphics& gfx) const {
  auto x = rect_.left;
  for (auto cell : cells_) {
    gfx::RectF rect(x, rect_.top, x + cell->width(),
                    ::ceilf(rect_.top + cell->line_height()));
    cell->Render(gfx, rect);
    x = rect.right;
  }
  gfx.Flush();
}

}  // namespace rendering
}  // namespace views

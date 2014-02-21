// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_cell.h"

#include "base/logging.h"

namespace views {
namespace rendering {

namespace {
inline gfx::ColorF ColorToColorF(Color color) {
  COLORREF const cr = color;
  return gfx::ColorF(
      static_cast<float>(GetRValue(cr)) / 255,
      static_cast<float>(GetGValue(cr)) / 255,
      static_cast<float>(GetBValue(cr)) / 255);
}

inline void FillRect(const gfx::Graphics& gfx, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(gfx, color);
  gfx.FillRectangle(fill_brush, rect);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Cell
//
Cell::Cell(Color cr, float cx, float cy)
    : m_crBackground(cr),
      m_cx(cx),
      m_cy(cy) {
  DCHECK_GE(cx, 1.0f);
  DCHECK_GE(cy, 1.0f);
}

Cell::Cell(const Cell& other)
    : Cell(other.m_crBackground, other.m_cx, other.m_cy) {
}

Cell::~Cell() {
}
void Cell::FillBackground(const gfx::Graphics& gfx,
                          const gfx::RectF& rect) const {
  FillRect(gfx, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                           ::ceilf(rect.bottom)),
           ColorToColorF(m_crBackground));
}

// rendering::Cell
bool Cell::Equal(const Cell* other) const {
  return other->class_name() == class_name() && other->m_cx == m_cx &&
         other->m_cy == m_cy && other->m_crBackground.Equal(m_crBackground);
}

Posn Cell::Fix(float iHeight, float) {
  m_cy = iHeight;
  return -1;
}

float Cell::GetDescent() const { return 0; }

uint Cell::Hash() const {
  auto nHash = static_cast<uint>(m_cx);
  nHash ^= static_cast<uint>(m_cy);
  nHash ^= m_crBackground.Hash();
  return nHash;
}

float Cell::MapPosnToX(const gfx::Graphics&, Posn) const {
  return -1.0f;
}

Posn Cell::MapXToPosn(const gfx::Graphics&, float) const {
  return -1;
}

bool Cell::Merge(Font*, Color, Color, TextDecoration, float) {
  return false;
}

void Cell::Render(const gfx::Graphics& gfx, const gfx::RectF& rect) const {
  FillBackground(gfx, rect);
}

}  // namespace rendering
}  // namespace views

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_cell.h"

#include "base/logging.h"

namespace views {
namespace rendering {

namespace {
float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

inline gfx::ColorF ColorToColorF(Color color) {
  COLORREF const cr = color;
  return gfx::ColorF(
      static_cast<float>(GetRValue(cr)) / 255,
      static_cast<float>(GetGValue(cr)) / 255,
      static_cast<float>(GetBValue(cr)) / 255);
}

inline void DrawLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                     float sx, float sy, float ex, float ey) {
  gfx.DrawLine(brush, sx, sy, ex, ey);
}

inline void DrawHLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                      float sx, float ex, float y) {
  DrawLine(gfx, brush, sx, y, ex, y);
}

void DrawText(const gfx::Graphics& gfx, const Font& font,
              const gfx::Brush& text_brush, const gfx::RectF& rect,
              const base::string16& string) {
  font.DrawText(gfx, text_brush, rect, string);
  gfx.Flush();
}

inline void DrawVLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                      float x, float sy, float ey) {
  DrawLine(gfx, brush, x, sy, x, ey);
}

inline void FillRect(const gfx::Graphics& gfx, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(gfx, color);
  gfx.FillRectangle(fill_brush, rect);
}

float FloorWidthToPixel(const gfx::Graphics& gfx, float width) {
  return gfx.FloorToPixel(gfx::SizeF(width, 0.0f)).width;
}

} // namespace

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

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
FillerCell::FillerCell(Color bgcolor, float width, float height)
    : Cell(bgcolor, width, height) {
}

FillerCell::FillerCell(const FillerCell& other)
    : Cell(other) {
}

FillerCell::~FillerCell() {
}

Cell* FillerCell::Copy() const {
  return new FillerCell(*this);
}

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//

MarkerCell::MarkerCell(Color crColor, Color bgcolor, float cx, float iHeight,
                       float iDescent, Posn lPosn, Kind eKind)
    : Cell(bgcolor, cx, iHeight),
      m_lStart(lPosn),
      m_lEnd(eKind == Kind_Wrap ? lPosn : lPosn + 1),
      m_crColor(crColor),
      m_iAscent(iHeight - iDescent),
      m_iDescent(iDescent),
      m_eKind(eKind) {
}

MarkerCell::MarkerCell(const MarkerCell& other)
    : Cell(other),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_crColor(other.m_crColor),
      m_iAscent(other.m_iAscent),
      m_iDescent(other.m_iDescent),
      m_eKind(other.m_eKind) {
}

MarkerCell::~MarkerCell() {
}

// rendering::Cell
Cell* MarkerCell::Copy() const {
  return new MarkerCell(*this);
}

bool MarkerCell::Equal(const Cell* other) const {
  if (!Cell::Equal(other))
    return false;
  auto const marker_cell = other->as<MarkerCell>();
  return m_crColor.Equal(marker_cell->m_crColor) &&
         m_eKind == marker_cell->m_eKind;
}

Posn MarkerCell::Fix(float iHeight, float iDescent) {
  m_cy = iHeight;
  m_iDescent = iDescent;
  return m_lEnd;
}

float MarkerCell::GetDescent() const {
  return m_iDescent;
}

uint MarkerCell::Hash() const {
  auto nHash = Cell::Hash();
  nHash <<= 8;
  nHash ^= m_crColor.Hash();
  nHash <<= 8;
  nHash ^= m_eKind;
  return nHash;
}

float MarkerCell::MapPosnToX(const gfx::Graphics&, Posn lPosn) const {
  if (lPosn < m_lStart)
    return -1.0f;
  if (lPosn >= m_lEnd)
    return -1.0f;
  return 0.0f;
}

Posn MarkerCell::MapXToPosn(const gfx::Graphics&, float) const {
  return m_lStart;
}

void MarkerCell::Render(const gfx::Graphics& gfx,
                        const gfx::RectF& rect) const {
  FillBackground(gfx, rect);

  auto const yBottom = rect.bottom - m_iDescent;
  auto const yTop = yBottom - m_iAscent;
  auto const xLeft = rect.left;
  auto const xRight = rect.right;

  gfx::Brush stroke_brush(gfx, ColorToColorF(m_crColor));

  switch (m_eKind) {
    case Kind_Eob: { // Draw <-
      // FIXME 2007-06-13 We should get internal leading from font.
      auto const iInternalLeading = 3;
      auto const w = std::max(m_iAscent / 6, 2.0f);
      auto const y = yBottom - (m_iAscent - iInternalLeading) / 2;
      DrawHLine(gfx, stroke_brush, xLeft, xRight, y);
      DrawLine(gfx, stroke_brush, xLeft + w, y - w, xLeft, y);
      DrawLine(gfx, stroke_brush, xLeft + w, y + w, xLeft, y);
      break;
    }

    case Kind_Eol: { // Draw V
      auto const y = yBottom - m_iAscent * 3 / 5;
      auto const w = std::max(m_cx / 6, 2.0f);
      auto const x = xLeft + m_cx / 2;
      DrawVLine(gfx, stroke_brush, x, y, yBottom);
      DrawLine(gfx, stroke_brush, x - w, yBottom - w, x, yBottom);
      DrawLine(gfx, stroke_brush, x + w, yBottom - w, x, yBottom);
      break;
    }

    case Kind_Tab: { // Draw |_|
      auto const w = std::max(m_iAscent / 6, 2.0f);
      DrawHLine(gfx, stroke_brush, xLeft + 2, xRight - 3, yBottom);
      DrawVLine(gfx, stroke_brush, xLeft + 2, yBottom, yBottom - w * 2);
      DrawVLine(gfx, stroke_brush, xRight - 3, yBottom, yBottom - w * 2);
      break;
    }

    case Kind_Wrap: { // Draw ->
      auto const ex = xRight - 1;
      auto const w = std::max(m_iAscent / 6, 2.0f);
      auto const y = yTop + m_iAscent / 2;
      DrawHLine(gfx, stroke_brush, xLeft, ex, y);
      DrawLine(gfx, stroke_brush, ex - w, y - w, xRight, y);
      DrawLine(gfx, stroke_brush, ex - w, y + w, xRight, y);
      break;
    }

      default:
      CAN_NOT_HAPPEN();
  }
}

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
TextCell::TextCell(const gfx::Graphics& gfx, const StyleValues* pStyle,
                   Color crColor, Color crBackground, Font* pFont, float cx,
                   Posn lPosn, const base::string16& characters)
    : Cell(crBackground, cx, AlignHeightToPixel(gfx, pFont->height())),
      m_crColor(crColor),
      m_eDecoration(pStyle->GetDecoration()),
      m_lStart(lPosn),
      m_lEnd(lPosn + 1),
      m_pFont(pFont),
      characters_(characters) {
}

TextCell::TextCell(const TextCell& other)
    : Cell(other),
      m_crColor(other.m_crColor),
      m_eDecoration(other.m_eDecoration),
      m_iDescent(other.m_iDescent),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_pFont(other.m_pFont),
      characters_(other.characters_) {
}

TextCell::~TextCell() {
}

void TextCell::AddChar(base::char16 char_code) {
  characters_.push_back(char_code);
}

// rendering::Cell
Cell* TextCell::Copy() const {
  return new TextCell(*this);
}

bool TextCell::Equal(const Cell* pCell) const {
  if (!Cell::Equal(pCell))
    return false;
  // reinterpret_cast used between related classes: 'class1' and 'class2'
  #pragma warning(suppress: 4946)
  auto const pText = reinterpret_cast<const TextCell*>(pCell);
  if (!m_crColor.Equal(pText->m_crColor))
    return false;
  if (m_eDecoration != pText->m_eDecoration)
    return false;
  return characters_ == pText->characters_;
}

Posn TextCell::Fix(float iHeight, float iDescent) {
    ASSERT(m_lStart <= m_lEnd);
    m_cy = iHeight;
    m_iDescent = iDescent;
    return m_lEnd;
}

float TextCell::GetDescent() const {
  return m_pFont->descent();
}

uint TextCell::Hash() const {
  uint nHash = Cell::Hash();
  nHash ^= m_crColor.Hash();
  nHash ^= m_pFont->Hash();
  nHash ^= m_eDecoration;
  nHash ^= static_cast<uint32_t>(characters_.length());
  for (auto ch : characters_) {
    nHash <<= 5;
    nHash ^= ch;
    nHash >>= 3;
  }
  return nHash;
}

float TextCell::MapPosnToX(const gfx::Graphics& gfx, Posn lPosn) const {
  if (lPosn < m_lStart)
    return -1;
  if (lPosn >= m_lEnd)
    return -1;
  auto const cwch = static_cast<size_t>(lPosn - m_lStart);
  if (!cwch)
    return 0;
  auto const width = m_pFont->GetTextWidth(characters_.data(), cwch);
  return FloorWidthToPixel(gfx, width);
}

Posn TextCell::MapXToPosn(const gfx::Graphics& gfx, float x) const {
  if (x >= m_cx)
    return m_lEnd;
  for (auto k = 1u; k <= characters_.length(); ++k) {
    auto const cx = FloorWidthToPixel(gfx,
      m_pFont->GetTextWidth(characters_.data(), k));
    if (x < cx)
      return static_cast<Posn>(m_lStart + k - 1);
  }
  return m_lEnd;
}

bool TextCell::Merge(Font* pFont, Color crColor, Color crBackground,
                    TextDecoration eDecoration, float cx) {
  if (m_pFont != pFont) return false;
  if (m_crColor != crColor) return false;
  if (m_crBackground != crBackground) return false;
  if (m_eDecoration != eDecoration) return false;
  m_cx += cx;
  m_lEnd += 1;
  return true;
}

void TextCell::Render(const gfx::Graphics& gfx, const gfx::RectF& rect) const {
  DCHECK(!characters_.empty());
  FillBackground(gfx, rect);
  gfx::Brush text_brush(gfx, ColorToColorF(m_crColor));
  DrawText(gfx, *m_pFont, text_brush, rect, characters_);

  auto const y = rect.bottom - m_iDescent -
                 (m_eDecoration != TextDecoration_None ? 1 : 0);
  #if SUPPORT_IME
  switch (m_eDecoration) {
    case TextDecoration_ImeInput:
      // TODO: We should use dotted line. It was PS_DOT.
      DrawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
      break;

    case TextDecoration_ImeInactiveA:
      DrawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
      break;

    case TextDecoration_ImeInactiveB:
      DrawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
      break;

    case TextDecoration_ImeActive:
      DrawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
      DrawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 2);
      break;

    case TextDecoration_None:
      break;

    case TextDecoration_GreenWave:
      // TODO: Implement TextDecoration_RedWave
      break;

    case TextDecoration_RedWave:
      // TODO: Implement TextDecoration_RedWave
      break;

    case TextDecoration_Underline:
      // TODO: Implement TextDecoration_Underline
      break;
  }
  #endif
}

} // namespace rendering
} // namespace views

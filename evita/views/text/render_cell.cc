// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_cell.h"

#include "base/at_exit.h"
#include "base/logging.h"
#include "evita/gfx/stroke_style.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

namespace {

// TODO(yosi) We should have Direct2D device in-dependent manager for
// controlling life time of resouces.
void DestroyStrokeStyleCallback(void* pointer) {
  delete reinterpret_cast<gfx::StrokeStyle*>(pointer);
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

void DrawWave(const gfx::Graphics& gfx, const gfx::Brush& brush, float sx,
              float ex, float y) {
  CR_DEFINE_STATIC_LOCAL(gfx::StrokeStyle, dash_style1, ());
  CR_DEFINE_STATIC_LOCAL(gfx::StrokeStyle, dash_style2, ());
  if (!dash_style1.is_realized()) {
    auto const dash_size = 2.0f;

    dash_style1.set_dash_style(gfx::DashStyle::Custom);
    dash_style1.set_dashes({dash_size, dash_size});
    dash_style1.Realize();
    base::AtExitManager::RegisterCallback(DestroyStrokeStyleCallback,
                                          &dash_style1);

    dash_style2.set_dash_style(gfx::DashStyle::Custom);
    dash_style2.set_dashes({dash_size, dash_size});
    dash_style2.set_dash_offset(dash_size);
    dash_style2.Realize();
    base::AtExitManager::RegisterCallback(DestroyStrokeStyleCallback,
                                          &dash_style2);
  }

  auto const stroke_width = 1.0f;
  gfx->DrawLine(gfx::PointF(sx, y + 1), gfx::PointF(ex, y + 1), brush,
                stroke_width, dash_style1);
  gfx->DrawLine(gfx::PointF(sx, y + 2), gfx::PointF(ex, y + 2), brush,
                stroke_width, dash_style2);
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
Cell::Cell(const RenderStyle& style, float cx, float cy)
    : m_cx(cx), m_cy(cy), style_(style) {
  DCHECK_GE(cx, 1.0f);
  DCHECK_GE(cy, 1.0f);
}

Cell::Cell(const Cell& other)
    : Cell(other.style_, other.m_cx, other.m_cy) {
}

Cell::~Cell() {
}

gfx::SizeF Cell::size() const {
  return gfx::SizeF(m_cx, m_cy);
}

void Cell::FillBackground(const gfx::Graphics& gfx,
                          const gfx::RectF& rect) const {
  FillRect(gfx, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                           ::ceilf(rect.bottom)),
           style_.bgcolor());
}

void Cell::FillOverlay(const gfx::Graphics& gfx,
                       const gfx::RectF& rect) const {
  if (style_.overlay_color().alpha() == 0.0f)
    return;
  FillRect(gfx, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                           ::ceilf(rect.bottom)),
           style_.overlay_color());
}

// rendering::Cell
bool Cell::Equal(const Cell* other) const {
  return other->class_name() == class_name() && other->m_cx == m_cx &&
         other->m_cy == m_cy && other->style_ == style_;
}

Posn Cell::Fix(float iHeight, float) {
  m_cy = iHeight;
  return -1;
}

float Cell::GetDescent() const { return 0; }

uint Cell::Hash() const {
  auto nHash = static_cast<uint>(m_cx);
  nHash ^= static_cast<uint>(m_cy);
  nHash ^= std::hash<RenderStyle>()(style_);
  return nHash;
}

gfx::RectF Cell::HitTestTextPosition(Posn) const {
  return gfx::RectF();
}

Posn Cell::MapXToPosn(const gfx::Graphics&, float) const {
  return -1;
}

bool Cell::Merge(const RenderStyle&, float) {
  return false;
}

void Cell::Render(const gfx::Graphics& gfx, const gfx::RectF& rect) const {
  FillBackground(gfx, rect);
}

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
FillerCell::FillerCell(const RenderStyle& style, float width, float height)
    : Cell(style, width, height) {
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

MarkerCell::MarkerCell(const RenderStyle& style, float width, float height,
                       Posn lPosn, TextMarker marker_name)
    : Cell(style, width, height),
      m_lStart(lPosn),
      m_lEnd(marker_name == TextMarker::LineWrap ? lPosn : lPosn + 1),
      m_iAscent(height - style.font()->descent()),
      m_iDescent(style.font()->descent()),
      marker_name_(marker_name) {
}

MarkerCell::MarkerCell(const MarkerCell& other)
    : Cell(other),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_iAscent(other.m_iAscent),
      m_iDescent(other.m_iDescent),
      marker_name_(other.marker_name_) {
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
  return marker_name_ == marker_cell->marker_name_;
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
  nHash ^= static_cast<int>(marker_name_);
  return nHash;
}

gfx::RectF MarkerCell::HitTestTextPosition(Posn lPosn) const {
  if (lPosn < m_lStart || lPosn >= m_lEnd)
    return gfx::RectF();
  return gfx::RectF(gfx::Point(), size());
}

Posn MarkerCell::MapXToPosn(const gfx::Graphics&, float) const {
  return m_lStart;
}

void MarkerCell::Render(const gfx::Graphics& gfx,
                        const gfx::RectF& rect) const {
  Cell::Render(gfx, rect);

  auto const yBottom = rect.bottom - m_iDescent;
  auto const yTop = yBottom - m_iAscent;
  auto const xLeft = rect.left;
  auto const xRight = rect.right;

  gfx::Brush stroke_brush(gfx, style().color());

  switch (marker_name_) {
    case TextMarker::EndOfDocument: { // Draw <-
      // FIXME 2007-06-13 We should get internal leading from font.
      auto const iInternalLeading = 3;
      auto const w = std::max(m_iAscent / 6, 2.0f);
      auto const y = yBottom - (m_iAscent - iInternalLeading) / 2;
      DrawHLine(gfx, stroke_brush, xLeft, xRight, y);
      DrawLine(gfx, stroke_brush, xLeft + w, y - w, xLeft, y);
      DrawLine(gfx, stroke_brush, xLeft + w, y + w, xLeft, y);
      break;
    }

    case TextMarker::EndOfLine: { // Draw V
      auto const y = yBottom - m_iAscent * 3 / 5;
      auto const w = std::max(m_cx / 6, 2.0f);
      auto const x = xLeft + m_cx / 2;
      DrawVLine(gfx, stroke_brush, x, y, yBottom);
      DrawLine(gfx, stroke_brush, x - w, yBottom - w, x, yBottom);
      DrawLine(gfx, stroke_brush, x + w, yBottom - w, x, yBottom);
      break;
    }


    case TextMarker::LineWrap: { // Draw ->
      auto const ex = xRight - 1;
      auto const w = std::max(m_iAscent / 6, 2.0f);

      auto const y = yTop + m_iAscent / 2;
      DrawHLine(gfx, stroke_brush, xLeft, ex, y);
      DrawLine(gfx, stroke_brush, ex - w, y - w, xRight, y);
      DrawLine(gfx, stroke_brush, ex - w, y + w, xRight, y);
      break;
    }

    case TextMarker::Tab: { // Draw |_|
      auto const w = std::max(m_iAscent / 6, 2.0f);
      DrawHLine(gfx, stroke_brush, xLeft + 2, xRight - 3, yBottom);
      DrawVLine(gfx, stroke_brush, xLeft + 2, yBottom, yBottom - w * 2);
      DrawVLine(gfx, stroke_brush, xRight - 3, yBottom, yBottom - w * 2);
      break;
    }
  }
  FillOverlay(gfx, rect);
}

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
TextCell::TextCell(const RenderStyle& style, float width, float height,
                   Posn lPosn, const base::string16& characters)
    : Cell(style, width, height),
      m_iDescent(style.font()->descent()),
      m_lStart(lPosn),
      m_lEnd(lPosn + 1),
      characters_(characters) {
}

TextCell::TextCell(const TextCell& other)
    : Cell(other),
      m_iDescent(other.m_iDescent),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
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

bool TextCell::Equal(const Cell* other) const {
  if (!Cell::Equal(other))
    return false;
  return characters_ == other->as<TextCell>()->characters_;
}

Posn TextCell::Fix(float iHeight, float iDescent) {
    ASSERT(m_lStart <= m_lEnd);
    m_cy = iHeight;
    m_iDescent = iDescent;
    return m_lEnd;
}

float TextCell::GetDescent() const {
  return style().font()->descent();
}

uint TextCell::Hash() const {
  return (Cell::Hash() << 3) ^ std::hash<base::string16>()(characters_);
}

gfx::RectF TextCell::HitTestTextPosition(Posn offset) const {
  if (offset < m_lStart || offset >= m_lEnd)
    return gfx::RectF();
  auto const cwch = static_cast<size_t>(offset - m_lStart);
  auto const left = style().font()->GetTextWidth(characters_.data(), cwch);
  auto const right = style().font()->GetTextWidth(characters_.data(), cwch + 1);
  return gfx::RectF(left, 0.0f, right, height());
}

Posn TextCell::MapXToPosn(const gfx::Graphics& gfx, float x) const {
  if (x >= m_cx)
    return m_lEnd;
  for (auto k = 1u; k <= characters_.length(); ++k) {
    auto const cx = FloorWidthToPixel(gfx,
      style().font()->GetTextWidth(characters_.data(), k));
    if (x < cx)
      return static_cast<Posn>(m_lStart + k - 1);
  }
  return m_lEnd;
}

bool TextCell::Merge(const RenderStyle& style, float width) {
  if (this->style() != style)
    return false;
  m_cx += width;
  m_lEnd += 1;
  return true;
}

void TextCell::Render(const gfx::Graphics& gfx, const gfx::RectF& rect) const {
  DCHECK(!characters_.empty());
  FillBackground(gfx, rect);
  gfx::Brush text_brush(gfx, style().color());
  DrawText(gfx, *style().font(), text_brush, rect, characters_);

  auto const baseline = rect.bottom - m_iDescent;

  #if SUPPORT_IME
  switch (style().text_decoration()) {
    case css::TextDecoration::ImeInput:
      DrawWave(gfx, text_brush, rect.left, rect.right, baseline + 1);
      break;

    case css::TextDecoration::ImeInactiveA:
      DrawHLine(gfx, text_brush, rect.left, rect.right, baseline + 1);
      break;

    case css::TextDecoration::ImeInactiveB:
      DrawHLine(gfx, text_brush, rect.left, rect.right, baseline + 1);
      break;

    case css::TextDecoration::ImeActive:
      DrawHLine(gfx, text_brush, rect.left, rect.right, baseline + 1);
      DrawHLine(gfx, text_brush, rect.left, rect.right, baseline + 2);
      break;

    case css::TextDecoration::None:
      break;

    case css::TextDecoration::GreenWave:
      DrawWave(gfx, gfx::Brush(gfx, gfx::ColorF::Green), rect.left,
               rect.right, baseline + 1);
      break;

    case css::TextDecoration::RedWave:
      DrawWave(gfx, gfx::Brush(gfx, gfx::ColorF::Red), rect.left,
               rect.right, baseline + 1);
      break;

    case css::TextDecoration::Underline:
      DrawHLine(gfx, text_brush, rect.left, rect.right, baseline + 1);
      break;
  }
  #endif
  FillOverlay(gfx, rect);
}

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
UnicodeCell::UnicodeCell(const RenderStyle& style, float width, float height,
                         Posn lPosn, const base::string16& characters)
    : TextCell(style, width, height + 4.0f, lPosn, characters) {
}

UnicodeCell::UnicodeCell(const UnicodeCell& other)
    : TextCell(other) {
}

UnicodeCell::~UnicodeCell() {
}

// rendering::Cell
Cell* UnicodeCell::Copy() const {
  return new UnicodeCell(*this);
}

bool UnicodeCell::Merge(const RenderStyle&, float) {
  return false;
}

void UnicodeCell::Render(const gfx::Graphics& gfx,
                         const gfx::RectF& rect) const {
  FillBackground(gfx, rect);


  gfx::Brush text_brush(gfx, style().color());
  DrawText(gfx, *style().font(), text_brush, rect, characters());

  gfx.DrawRectangle(text_brush,
                    gfx::RectF(rect.left, rect.top,
                               rect.right - 1, rect.bottom - 1));
  FillOverlay(gfx, rect);
}

} // namespace rendering
} // namespace views

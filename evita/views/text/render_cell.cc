// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_cell.h"

#include "base/logging.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

namespace {

void DrawText(gfx::Canvas* canvas, const Font& font,
              const gfx::Brush& text_brush, const gfx::RectF& rect,
              const base::string16& string) {
  font.DrawText(canvas, text_brush, rect, string);
  canvas->Flush();
}

inline void FillRect(gfx::Canvas* canvas, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(canvas, color);
  canvas->FillRectangle(fill_brush, rect);
}

} // namespace

//////////////////////////////////////////////////////////////////////
//
// Cell
//
Cell::Cell(const RenderStyle& style, float width, float height, float descent)
    : descent_(descent), height_(height), line_height_(height), style_(style),
      width_(width) {
  DCHECK_GE(width, 1.0f);
  DCHECK_GE(height, 1.0f);
}

Cell::Cell(const Cell& other)
    : Cell(other.style_, other.width_, other.height_, other.descent_) {
  line_descent_ = other.line_descent_;
  line_height_ = other.line_height_;
}

Cell::~Cell() {
}

float Cell::top() const {
  return line_height() - line_descent() - height() + descent();
}

void Cell::FillBackground(gfx::Canvas* canvas,
                          const gfx::RectF& rect) const {
  FillRect(canvas, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                           ::ceilf(rect.bottom)),
           style_.bgcolor());
}

void Cell::FillOverlay(gfx::Canvas* canvas,
                       const gfx::RectF& rect) const {
  if (style_.overlay_color().alpha() == 0.0f)
    return;
  FillRect(canvas, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                           ::ceilf(rect.bottom)),
           style_.overlay_color());
}

void Cell::IncrementWidth(float amount) {
  width_ += amount;
}

// rendering::Cell
bool Cell::Equal(const Cell* other) const {
  return other->class_name() == class_name() && other->width_ == width_ &&
         other->line_height_ == line_height_ && other->style_ == style_;
}

Posn Cell::Fix(float line_height, float line_descent) {
  line_descent_ = line_descent;
  line_height_ = line_height;
  return -1;
}

uint32_t Cell::Hash() const {
  auto nHash = static_cast<uint>(width_);
  nHash ^= static_cast<uint>(line_height_);
  nHash ^= std::hash<RenderStyle>()(style_);
  return nHash;
}

gfx::RectF Cell::HitTestTextPosition(Posn) const {
  return gfx::RectF();
}

Posn Cell::MapXToPosn(float) const {
  return -1;
}

bool Cell::Merge(const RenderStyle&, float) {
  return false;
}

void Cell::Render(gfx::Canvas* canvas, const gfx::RectF& rect) const {
  FillBackground(canvas, rect);
}

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
FillerCell::FillerCell(const RenderStyle& style, float width, float height)
    : Cell(style, width, height, 0.0f) {
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
// WithFont
//
WithFont::WithFont(const Font& font) : font_(&font) {
}

WithFont::WithFont(const WithFont& other) : font_(other.font_) {
}

WithFont::~WithFont() {
}

float WithFont::underline() const {
  return font_->underline();
}

float WithFont::underline_thickness() const {
  return font_->underline_thickness();
}

void WithFont::DrawHLine(gfx::Canvas* canvas, const gfx::Brush& brush,
                         float sx, float ex, float y) const {
  canvas->DrawLine(brush, gfx::PointF(sx, y), gfx::PointF(ex, y),
                   font_->underline_thickness());
}

void WithFont::DrawLine(gfx::Canvas* canvas, const gfx::Brush& brush,
                        float sx, float sy, float ex, float ey,
                        float width) const {
  canvas->DrawLine(brush, gfx::PointF(sx, sy), gfx::PointF(ex, ey),
                   width* font_->underline_thickness());
}

void WithFont::DrawVLine(gfx::Canvas* canvas, const gfx::Brush& brush,
                         float x, float sy, float ey) const {
  canvas->DrawLine(brush, gfx::PointF(x, sy), gfx::PointF(x, ey),
                   font_->underline_thickness());
}

void WithFont::DrawWave(gfx::Canvas* canvas, const gfx::Brush& brush,
                        const gfx::RectF& bounds, float baseline) const {
  auto const wave = std::max(font_->underline() * 1.3f, 2.0f);
  auto const pen_width = font_->underline_thickness();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  for (auto x = bounds.left; x < bounds.right; x += wave) {
    auto const bottom = baseline + wave;
    auto const top = baseline;
    // top to bottom
    canvas->DrawLine(brush, gfx::PointF(x, top), gfx::PointF(x + wave, bottom),
                     pen_width);
    x += wave;
    // bottom to top
    canvas->DrawLine(brush, gfx::PointF(x, bottom), gfx::PointF(x + wave, top),
                     pen_width);
  }
}

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
MarkerCell::MarkerCell(const RenderStyle& style, float width, float height,
                       Posn lPosn, TextMarker marker_name)
    : Cell(style, width, height, style.font().descent()),
      WithFont(style.font()),
      end_(marker_name == TextMarker::LineWrap ? lPosn : lPosn + 1),
      marker_name_(marker_name), start_(lPosn) {
}

MarkerCell::MarkerCell(const MarkerCell& other)
    : Cell(other), WithFont(other),
      end_(other.end_), marker_name_(other.marker_name_), start_(other.start_) {
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

Posn MarkerCell::Fix(float line_height, float line_descent) {
  Cell::Fix(line_height, line_descent);
  return end_;
}

uint32_t MarkerCell::Hash() const {
  auto nHash = Cell::Hash();
  nHash <<= 8;
  nHash ^= static_cast<int>(marker_name_);
  return nHash;
}

gfx::RectF MarkerCell::HitTestTextPosition(Posn lPosn) const {
  if (lPosn < start_ || lPosn >= end_)
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(0.0f, top()), gfx::SizeF(width(), height()));
}

Posn MarkerCell::MapXToPosn(float) const {
  return start_;
}

// Render marker above baseline.
void MarkerCell::Render(gfx::Canvas* canvas,
                        const gfx::RectF& rect) const {
  Cell::Render(canvas, rect);

  auto const ascent = height() - descent();
  auto const marker_rect = gfx::RectF(gfx::PointF(rect.left, rect.top + top()),
                                      gfx::SizeF(width(), height()));
  gfx::Brush stroke_brush(canvas, style().color());
  auto const baseline = marker_rect.bottom - descent();
  switch (marker_name_) {
    case TextMarker::EndOfDocument: { // Draw <-
      auto const wing = underline() * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right;
      DrawHLine(canvas, stroke_brush, sx, ex, y);
      DrawLine(canvas, stroke_brush, sx + w, y - w, sx, y, 1.0f);
      DrawLine(canvas, stroke_brush, sx + w, y + w, sx, y, 1.0f);
      break;
    }

    case TextMarker::EndOfLine: { // Draw V
      auto const ey = baseline;
      auto const sy = ey - ascent * 3 / 5;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const x = marker_rect.left + width() / 2;
      DrawVLine(canvas, stroke_brush, x, sy, ey);
      DrawLine(canvas, stroke_brush, x - w, ey - w, x, ey, 1.0f);
      DrawLine(canvas, stroke_brush, x + w, ey - w, x, ey, 1.0f);
      break;
    }

    case TextMarker::LineWrap: { // Draw ->
      auto const wing = underline() * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right - underline_thickness();
      DrawHLine(canvas, stroke_brush, sx, ex, y);
      DrawLine(canvas, stroke_brush, ex - w, y - w, ex, y, 1.0f);
      DrawLine(canvas, stroke_brush, ex - w, y + w, ex, y, 1.0f);
      break;
    }

    case TextMarker::Tab: { // Draw |_|
      auto const sx = marker_rect.left + underline_thickness() * 2;
      auto const ex = marker_rect.right - underline_thickness() * 2;
      auto const y = baseline;
      auto const w = std::max(ascent / 6, 2.0f);
      DrawHLine(canvas, stroke_brush, sx, ex, y);
      DrawVLine(canvas, stroke_brush, sx, y, y - w * 2);
      DrawVLine(canvas, stroke_brush, ex, y, y - w * 2);
      break;
    }
  }
  FillOverlay(canvas, marker_rect);
}

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
TextCell::TextCell(const RenderStyle& style, float width, float height,
                   Posn lPosn, const base::string16& characters)
    : Cell(style, width, height, style.font().descent()),
      WithFont(style.font()),
      characters_(characters), end_(lPosn + 1), start_(lPosn) {
}

TextCell::TextCell(const TextCell& other)
    : Cell(other), WithFont(other),
      characters_(other.characters_), end_(other.end_), start_(other.start_) {
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

Posn TextCell::Fix(float line_height, float descent) {
  Cell::Fix(line_height, descent);
  DCHECK_LT(start_, end_);
  return end_;
}

uint32_t TextCell::Hash() const {
  return (Cell::Hash() << 3) ^ std::hash<base::string16>()(characters_);
}

// Returns bounds rectangle of caret at |offset|. Caret is placed before
// character at |offset|. So, height of caret is height of character before
// |offset|.
gfx::RectF TextCell::HitTestTextPosition(Posn offset) const {
  if (offset < start_ || offset > end_)
    return gfx::RectF();
  auto const length = static_cast<size_t>(offset - start_);
  auto const left = length ?
      style().font().GetTextWidth(characters_.data(), length) : 0.0f;
  return gfx::RectF(gfx::PointF(left, top()),
                    gfx::SizeF(1.0f, height()));
}

Posn TextCell::MapXToPosn(float x) const {
  if (x >= width())
    return end_;
  for (auto k = 1u; k <= characters_.length(); ++k) {
    auto const cx = style().font().GetTextWidth(characters_.data(), k);
    if (x < cx)
      return static_cast<Posn>(start_ + k - 1);
  }
  return end_;
}

bool TextCell::Merge(const RenderStyle& style, float width) {
  if (this->style() != style)
    return false;
  IncrementWidth(width);
  end_ += 1;
  return true;
}

void TextCell::Render(gfx::Canvas* canvas, const gfx::RectF& rect) const {
  DCHECK(!characters_.empty());
  auto const text_rect = gfx::RectF(
      gfx::PointF(rect.left, rect.top + top()), 
      gfx::SizeF(rect.width(), height()));
  FillBackground(canvas, rect);
  gfx::Brush text_brush(canvas, style().color());
  DrawText(canvas, style().font(), text_brush, text_rect, characters_);

  auto const baseline = text_rect.bottom - descent();
  auto const underline = baseline + this->underline();
  switch (style().text_decoration()) {
    case css::TextDecoration::ImeInput:
      DrawWave(canvas, text_brush, rect, underline);
      break;

    case css::TextDecoration::ImeInactiveA:
      DrawHLine(canvas, text_brush, rect.left, rect.right, underline);
      break;

    case css::TextDecoration::ImeInactiveB:
      DrawHLine(canvas, text_brush, rect.left, rect.right, underline);
      break;

    case css::TextDecoration::ImeActive:
      DrawLine(canvas, text_brush, rect.left, underline,
               rect.right, underline, 2.0f);
      break;

    case css::TextDecoration::None:
      break;

    case css::TextDecoration::GreenWave:
      DrawWave(canvas, gfx::Brush(canvas, gfx::ColorF::Green), rect, baseline);
      break;

    case css::TextDecoration::RedWave:
      DrawWave(canvas, gfx::Brush(canvas, gfx::ColorF::Red), rect, baseline);
      break;

    case css::TextDecoration::Underline:
      DrawHLine(canvas, text_brush, rect.left, rect.right, underline);
      break;
  }

  FillOverlay(canvas, text_rect);
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

gfx::RectF UnicodeCell::HitTestTextPosition(Posn offset) const {
  if (offset < start() || offset > end())
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(width(), top()), gfx::SizeF(1.0f, height()));
}

bool UnicodeCell::Merge(const RenderStyle&, float) {
  return false;
}

void UnicodeCell::Render(gfx::Canvas* canvas, const gfx::RectF& rect) const {
  auto const text_rect = gfx::RectF(
      gfx::PointF(rect.left, rect.top + top()),
      gfx::SizeF(rect.width(), height())) - gfx::SizeF(1, 1);
  FillBackground(canvas, rect);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, text_rect);
  gfx::Brush text_brush(canvas, style().color());
  DrawText(canvas, style().font(), text_brush, text_rect - gfx::SizeF(1, 1),
           characters());
  canvas->DrawRectangle(text_brush, text_rect);
  FillOverlay(canvas, text_rect);
}

} // namespace rendering
} // namespace views

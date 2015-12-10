// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/paint/inline_box_painter.h"

#include "evita/gfx_base.h"
#include "evita/views/text/inline_box.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_style.h"

namespace paint {

using Font = views::rendering::Font;
using InlineBox = views::rendering::InlineBox;
using InlineFillerBox = views::rendering::InlineFillerBox;
using InlineMarkerBox = views::rendering::InlineMarkerBox;
using InlineTextBox = views::rendering::InlineTextBox;
using InlineUnicodeBox = views::rendering::InlineUnicodeBox;
using TextMarker = views::rendering::TextMarker;

namespace {

//////////////////////////////////////////////////////////////////////
//
// LinePainer
//
class LinePainer {
 public:
  explicit LinePainer(gfx::Canvas* canvas, const Font& font);
  ~LinePainer();

  void DrawHLine(const gfx::Brush& brush, float sx, float sy, float y) const;
  void DrawLine(const gfx::Brush& brush,
                float x1,
                float y1,
                float x2,
                float y2,
                float width) const;
  void DrawVLine(const gfx::Brush& brush, float x, float sy, float ey) const;
  void DrawWave(const gfx::Brush& brush,
                const gfx::RectF& bounds,
                float baseline) const;

 private:
  gfx::Canvas* const canvas_;
  const Font& font_;

  DISALLOW_COPY_AND_ASSIGN(LinePainer);
};

LinePainer::LinePainer(gfx::Canvas* canvas, const Font& font)
    : canvas_(canvas), font_(font) {}

LinePainer::~LinePainer() {}

void LinePainer::DrawHLine(const gfx::Brush& brush,
                           float sx,
                           float ex,
                           float y) const {
  canvas_->DrawLine(brush, gfx::PointF(sx, y), gfx::PointF(ex, y),
                    font_.underline_thickness());
}

void LinePainer::DrawLine(const gfx::Brush& brush,
                          float sx,
                          float sy,
                          float ex,
                          float ey,
                          float width) const {
  canvas_->DrawLine(brush, gfx::PointF(sx, sy), gfx::PointF(ex, ey),
                    width * font_.underline_thickness());
}

void LinePainer::DrawVLine(const gfx::Brush& brush,
                           float x,
                           float sy,
                           float ey) const {
  canvas_->DrawLine(brush, gfx::PointF(x, sy), gfx::PointF(x, ey),
                    font_.underline_thickness());
}

void LinePainer::DrawWave(const gfx::Brush& brush,
                          const gfx::RectF& bounds,
                          float baseline) const {
  auto const wave = std::max(font_.underline() * 1.3f, 2.0f);
  auto const pen_width = font_.underline_thickness();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, bounds);
  for (auto x = bounds.left; x < bounds.right; x += wave) {
    auto const bottom = baseline + wave;
    auto const top = baseline;
    // top to bottom
    canvas_->DrawLine(brush, gfx::PointF(x, top), gfx::PointF(x + wave, bottom),
                      pen_width);
    x += wave;
    // bottom to top
    canvas_->DrawLine(brush, gfx::PointF(x, bottom), gfx::PointF(x + wave, top),
                      pen_width);
  }
}

void DrawText(gfx::Canvas* canvas,
              const Font& font,
              const gfx::Brush& text_brush,
              const gfx::RectF& rect,
              const base::string16& string) {
  font.DrawText(canvas, text_brush, rect, string);
}

void FillRect(gfx::Canvas* canvas, const gfx::RectF& rect, gfx::ColorF color) {
  gfx::Brush fill_brush(canvas, color);
  canvas->FillRectangle(fill_brush, rect);
}

void FillBackground(gfx::Canvas* canvas,
                    const gfx::RectF& rect,
                    const InlineBox& inline_box) {
  FillRect(canvas, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                              ::ceilf(rect.bottom)),
           inline_box.style().bgcolor());
}

void FillOverlay(gfx::Canvas* canvas,
                 const gfx::RectF& rect,
                 const InlineBox& inline_box) {
  if (inline_box.style().overlay_color().alpha() == 0.0f)
    return;
  FillRect(canvas, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                              ::ceilf(rect.bottom)),
           inline_box.style().overlay_color());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// InlineBoxPainter
//
InlineBoxPainter::InlineBoxPainter(gfx::Canvas* canvas, const gfx::RectF& rect)
    : canvas_(canvas), rect_(rect) {}

InlineBoxPainter::~InlineBoxPainter() {}

void InlineBoxPainter::Paint(const InlineBox& inline_box) {
  const_cast<InlineBox*>(&inline_box)->Accept(this);
}

// Implements InlineBoxVisitor member functions
void InlineBoxPainter::VisitInlineFillerBox(InlineFillerBox* inline_box) {
  FillBackground(canvas_, rect_, *inline_box);
}

// Paint marker above baseline.
void InlineBoxPainter::VisitInlineMarkerBox(InlineMarkerBox* inline_box) {
  FillBackground(canvas_, rect_, *inline_box);

  auto const ascent = inline_box->height() - inline_box->descent();
  auto const marker_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(inline_box->width(), inline_box->height()));
  gfx::Brush stroke_brush(canvas_, inline_box->style().color());
  auto const baseline = marker_rect.bottom - inline_box->descent();
  LinePainer line_painter(canvas_, inline_box->font());
  auto const underline = inline_box->font().underline();
  auto const underline_thickness = inline_box->font().underline_thickness();
  switch (inline_box->marker_name()) {
    case TextMarker::EndOfDocument: {  // Draw <-
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right;
      line_painter.DrawHLine(stroke_brush, sx, ex, y);
      line_painter.DrawLine(stroke_brush, sx + w, y - w, sx, y, 1.0f);
      line_painter.DrawLine(stroke_brush, sx + w, y + w, sx, y, 1.0f);
      break;
    }

    case TextMarker::EndOfLine: {  // Draw V
      auto const ey = baseline;
      auto const sy = ey - ascent * 3 / 5;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const x = marker_rect.left + inline_box->width() / 2;
      line_painter.DrawVLine(stroke_brush, x, sy, ey);
      line_painter.DrawLine(stroke_brush, x - w, ey - w, x, ey, 1.0f);
      line_painter.DrawLine(stroke_brush, x + w, ey - w, x, ey, 1.0f);
      break;
    }

    case TextMarker::LineWrap: {  // Draw ->
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right - underline_thickness;
      line_painter.DrawHLine(stroke_brush, sx, ex, y);
      line_painter.DrawLine(stroke_brush, ex - w, y - w, ex, y, 1.0f);
      line_painter.DrawLine(stroke_brush, ex - w, y + w, ex, y, 1.0f);
      break;
    }

    case TextMarker::Tab: {  // Draw |_|
      auto const sx = marker_rect.left + underline_thickness * 2;
      auto const ex = marker_rect.right - underline_thickness * 2;
      auto const y = baseline;
      auto const w = std::max(ascent / 6, 2.0f);
      line_painter.DrawHLine(stroke_brush, sx, ex, y);
      line_painter.DrawVLine(stroke_brush, sx, y, y - w * 2);
      line_painter.DrawVLine(stroke_brush, ex, y, y - w * 2);
      break;
    }
  }
  FillOverlay(canvas_, marker_rect, *inline_box);
}

void InlineBoxPainter::VisitInlineTextBox(InlineTextBox* inline_box) {
  DCHECK(!inline_box->characters().empty());
  auto const text_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(rect_.width(), inline_box->height()));
  FillBackground(canvas_, rect_, *inline_box);
  gfx::Brush text_brush(canvas_, inline_box->style().color());
  DrawText(canvas_, inline_box->style().font(), text_brush, text_rect,
           inline_box->characters());

  auto const baseline = text_rect.bottom - inline_box->descent();
  auto const underline = baseline + inline_box->font().underline();
  LinePainer line_painter(canvas_, inline_box->font());
  switch (inline_box->style().text_decoration()) {
    case css::TextDecoration::ImeInput:
      line_painter.DrawWave(text_brush, rect_, underline);
      break;

    case css::TextDecoration::ImeInactiveA:
      line_painter.DrawHLine(text_brush, rect_.left, rect_.right, underline);
      break;

    case css::TextDecoration::ImeInactiveB:
      line_painter.DrawHLine(text_brush, rect_.left, rect_.right, underline);
      break;

    case css::TextDecoration::ImeActive:
      line_painter.DrawLine(text_brush, rect_.left, underline, rect_.right,
                            underline, 2.0f);
      break;

    case css::TextDecoration::None:
      break;

    case css::TextDecoration::GreenWave:
      line_painter.DrawWave(gfx::Brush(canvas_, gfx::ColorF::Green), rect_,
                            baseline);
      break;

    case css::TextDecoration::RedWave:
      line_painter.DrawWave(gfx::Brush(canvas_, gfx::ColorF::Red), rect_,
                            baseline);
      break;

    case css::TextDecoration::Underline:
      line_painter.DrawHLine(text_brush, rect_.left, rect_.right, underline);
      break;
  }

  FillOverlay(canvas_, text_rect, *inline_box);
}

void InlineBoxPainter::VisitInlineUnicodeBox(InlineUnicodeBox* inline_box) {
  auto const text_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(rect_.width(), inline_box->height())) -
      gfx::SizeF(1, 1);
  const auto& style = inline_box->style();
  FillBackground(canvas_, rect_, *inline_box);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, text_rect);
  gfx::Brush text_brush(canvas_, style.color());
  DrawText(canvas_, style.font(), text_brush, text_rect - gfx::SizeF(1, 1),
           inline_box->characters());
  canvas_->DrawRectangle(text_brush, text_rect);
  FillOverlay(canvas_, text_rect, *inline_box);
}

}  // namespace paint

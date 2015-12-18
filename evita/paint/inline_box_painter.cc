// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/paint/inline_box_painter.h"

#include "evita/gfx_base.h"
#include "evita/layout/render_font.h"
#include "evita/layout/render_style.h"
#include "evita/paint/public/line/inline_box.h"
#include "evita/paint/public/line/inline_box_visitor.h"

namespace paint {

using Font = layout::Font;

namespace {

void DrawHLine(gfx::Canvas* canvas,
               const Font& font,
               const gfx::Brush& brush,
               float sx,
               float ex,
               float y) {
  canvas->DrawLine(brush, gfx::PointF(sx, y), gfx::PointF(ex, y),
                   font.underline_thickness());
}

void DrawLine(gfx::Canvas* canvas,
              const Font& font,
              const gfx::Brush& brush,
              float sx,
              float sy,
              float ex,
              float ey,
              float width) {
  canvas->DrawLine(brush, gfx::PointF(sx, sy), gfx::PointF(ex, ey),
                   width * font.underline_thickness());
}

void DrawVLine(gfx::Canvas* canvas,
               const Font& font,
               const gfx::Brush& brush,
               float x,
               float sy,
               float ey) {
  canvas->DrawLine(brush, gfx::PointF(x, sy), gfx::PointF(x, ey),
                   font.underline_thickness());
}

void DrawWave(gfx::Canvas* canvas,
              const Font& font,
              const gfx::Brush& brush,
              const gfx::RectF& bounds,
              float baseline) {
  auto const wave = std::max(font.underline() * 1.3f, 2.0f);
  auto const pen_width = font.underline_thickness();
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

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor
//
class PaintVisitor final : public InlineBoxVisitor {
 public:
  PaintVisitor(gfx::Canvas* canvas, const gfx::RectF& rect);
  ~PaintVisitor() final = default;

 private:
#define V(name) void Visit##name(name* inline_box) final;
  FOR_EACH_PAINT_INLINE_BOX(V)
#undef V

  gfx::Canvas* const canvas_;
  const gfx::RectF& rect_;

  DISALLOW_COPY_AND_ASSIGN(PaintVisitor);
};

PaintVisitor::PaintVisitor(gfx::Canvas* canvas, const gfx::RectF& rect)
    : canvas_(canvas), rect_(rect) {}

// Implements InlineBoxVisitor member functions
void PaintVisitor::VisitInlineFillerBox(InlineFillerBox* inline_box) {
  FillBackground(canvas_, rect_, *inline_box);
}

// Paint marker above baseline.
void PaintVisitor::VisitInlineMarkerBox(InlineMarkerBox* inline_box) {
  FillBackground(canvas_, rect_, *inline_box);

  auto const ascent = inline_box->height() - inline_box->descent();
  auto const marker_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(inline_box->width(), inline_box->height()));
  gfx::Brush stroke_brush(canvas_, inline_box->style().color());
  auto const baseline = marker_rect.bottom - inline_box->descent();
  const auto& font = inline_box->font();
  auto const underline = inline_box->font().underline();
  auto const underline_thickness = inline_box->font().underline_thickness();
  switch (inline_box->marker_name()) {
    case TextMarker::EndOfDocument: {  // Draw <-
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right;
      DrawHLine(canvas_, font, stroke_brush, sx, ex, y);
      DrawLine(canvas_, font, stroke_brush, sx + w, y - w, sx, y, 1.0f);
      DrawLine(canvas_, font, stroke_brush, sx + w, y + w, sx, y, 1.0f);
      break;
    }

    case TextMarker::EndOfLine: {  // Draw V
      auto const ey = baseline;
      auto const sy = ey - ascent * 3 / 5;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const x = marker_rect.left + inline_box->width() / 2;
      DrawVLine(canvas_, font, stroke_brush, x, sy, ey);
      DrawLine(canvas_, font, stroke_brush, x - w, ey - w, x, ey, 1.0f);
      DrawLine(canvas_, font, stroke_brush, x + w, ey - w, x, ey, 1.0f);
      break;
    }

    case TextMarker::LineWrap: {  // Draw ->
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right - underline_thickness;
      DrawHLine(canvas_, font, stroke_brush, sx, ex, y);
      DrawLine(canvas_, font, stroke_brush, ex - w, y - w, ex, y, 1.0f);
      DrawLine(canvas_, font, stroke_brush, ex - w, y + w, ex, y, 1.0f);
      break;
    }

    case TextMarker::Tab: {  // Draw |_|
      auto const sx = marker_rect.left + underline_thickness * 2;
      auto const ex = marker_rect.right - underline_thickness * 2;
      auto const y = baseline;
      auto const w = std::max(ascent / 6, 2.0f);
      DrawHLine(canvas_, font, stroke_brush, sx, ex, y);
      DrawVLine(canvas_, font, stroke_brush, sx, y, y - w * 2);
      DrawVLine(canvas_, font, stroke_brush, ex, y, y - w * 2);
      break;
    }
  }
  FillOverlay(canvas_, marker_rect, *inline_box);
}

void PaintVisitor::VisitInlineTextBox(InlineTextBox* inline_box) {
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
  const auto& font = inline_box->font();
  switch (inline_box->style().text_decoration()) {
    case css::TextDecoration::ImeInput:
      DrawWave(canvas_, font, text_brush, rect_, underline);
      break;

    case css::TextDecoration::ImeInactiveA:
      DrawHLine(canvas_, font, text_brush, rect_.left, rect_.right, underline);
      break;

    case css::TextDecoration::ImeInactiveB:
      DrawHLine(canvas_, font, text_brush, rect_.left, rect_.right, underline);
      break;

    case css::TextDecoration::ImeActive:
      DrawLine(canvas_, font, text_brush, rect_.left, underline, rect_.right,
               underline, 2.0f);
      break;

    case css::TextDecoration::None:
      break;

    case css::TextDecoration::GreenWave:
      DrawWave(canvas_, font, gfx::Brush(canvas_, gfx::ColorF::Green), rect_,
               baseline);
      break;

    case css::TextDecoration::RedWave:
      DrawWave(canvas_, font, gfx::Brush(canvas_, gfx::ColorF::Red), rect_,
               baseline);
      break;

    case css::TextDecoration::Underline:
      DrawHLine(canvas_, font, text_brush, rect_.left, rect_.right, underline);
      break;
  }

  FillOverlay(canvas_, text_rect, *inline_box);
}

void PaintVisitor::VisitInlineUnicodeBox(InlineUnicodeBox* inline_box) {
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

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// InlineBoxPainter
//
InlineBoxPainter::InlineBoxPainter(const InlineBox& inline_box)
    : inline_box_(inline_box) {}

InlineBoxPainter::~InlineBoxPainter() {}

void InlineBoxPainter::Paint(gfx::Canvas* canvas, const gfx::RectF& rect) {
  PaintVisitor visitor(canvas, rect);
  const_cast<InlineBox&>(inline_box_).Accept(&visitor);
}

}  // namespace paint

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/text/paint/inline_box_painter.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/font.h"
#include "evita/gfx/stroke_style.h"
#include "evita/text/paint/public/line/inline_box.h"
#include "evita/text/paint/public/line/inline_box_visitor.h"
#include "evita/text/style/computed_style.h"

namespace paint {

using TextDecorationLine = layout::TextDecorationLine;
using TextDecorationStyle = layout::TextDecorationStyle;

namespace {

void DrawHLine(gfx::Canvas* canvas,
               const gfx::Brush& brush,
               float sx,
               float ex,
               float y,
               float size) {
  const auto size2 = size / 2;
  canvas->DrawLine(brush, gfx::PointF(sx + size2, y + size2),
                   gfx::PointF(ex - size2, y + size2), size);
}

void DrawHLine(gfx::Canvas* canvas,
               const gfx::Brush& brush,
               float sx,
               float ex,
               float y,
               float stroke_width,
               const gfx::StrokeStyle& stroke_style) {
  const auto size2 = stroke_width / 2;
  (*canvas)->DrawLine(gfx::PointF(sx + size2, y + size2),
                      gfx::PointF(ex - size2, y + size2), brush, stroke_width,
                      stroke_style);
}

void DrawLine(gfx::Canvas* canvas,
              const gfx::Brush& brush,
              float sx,
              float sy,
              float ex,
              float ey,
              float width) {
  canvas->DrawLine(brush, gfx::PointF(sx, sy), gfx::PointF(ex, ey), width);
}

void DrawVLine(gfx::Canvas* canvas,
               const gfx::Brush& brush,
               float x,
               float sy,
               float ey,
               float size) {
  const auto size2 = size / 2;
  canvas->DrawLine(brush, gfx::PointF(x + size2, sy + size2),
                   gfx::PointF(x + size2, ey - size2), size);
}

void DrawWave(gfx::Canvas* canvas,
              const gfx::Brush& brush,
              const gfx::RectF& bounds,
              float underline,
              float wave,
              float size,
              float pen_width) {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  common::ComPtr<ID2D1PathGeometry> geometry;
  canvas->GetD2D1Factory()->CreatePathGeometry(&geometry);
  common::ComPtr<ID2D1GeometrySink> sink;
  geometry->Open(&sink);
  auto const bottom = underline + size;
  auto const top = underline - size;
  auto x = bounds.left - wave / 2;
  sink->BeginFigure(gfx::PointF(x, bottom), D2D1_FIGURE_BEGIN_HOLLOW);
  while (x < bounds.right) {
    x += wave;
    sink->AddLine(gfx::PointF(x, top));
    x += wave;
    sink->AddLine(gfx::PointF(x, bottom));
  }
  sink->EndFigure(D2D1_FIGURE_END_OPEN);
  sink->Close();
  gfx::StrokeStyle stroke_style;
  stroke_style.set_cap_style(gfx::CapStyle::Round);
  stroke_style.set_line_join(gfx::LineJoin::Round);
  stroke_style.Realize(canvas);
  (*canvas)->DrawGeometry(geometry, brush, pen_width, stroke_style);
}

void DrawText(gfx::Canvas* canvas,
              const gfx::Font& font,
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
  auto const underline = font.underline();
  auto const size = font.underline_thickness();
  switch (inline_box->marker_name()) {
    case TextMarker::EndOfDocument: {  // Draw <-
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right;
      DrawLine(canvas_, stroke_brush, sx, y, ex, y, size);
      DrawLine(canvas_, stroke_brush, sx + w, y - w, sx, y, size);
      DrawLine(canvas_, stroke_brush, sx + w, y + w, sx, y, size);
      break;
    }

    case TextMarker::EndOfLine: {  // Draw V
      auto const ey = baseline;
      auto const sy = ey - ascent * 3 / 5;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const x = marker_rect.left + inline_box->width() / 2;
      DrawLine(canvas_, stroke_brush, x, sy, x, ey, size);
      DrawLine(canvas_, stroke_brush, x - w, ey - w, x, ey, size);
      DrawLine(canvas_, stroke_brush, x + w, ey - w, x, ey, size);
      break;
    }

    case TextMarker::LineWrap: {  // Draw ->
      auto const wing = underline * 3;
      auto const w = std::max(ascent / 6, 2.0f);
      auto const y = baseline - (ascent - wing) / 2;
      auto const sx = marker_rect.left;
      auto const ex = marker_rect.right - size;
      DrawLine(canvas_, stroke_brush, sx, y, ex, y, size);
      DrawLine(canvas_, stroke_brush, ex - w, y - w, ex, y, size);
      DrawLine(canvas_, stroke_brush, ex - w, y + w, ex, y, size);
      break;
    }

    case TextMarker::Tab: {  // Draw |_|
      auto const sx = marker_rect.left + size * 2;
      auto const ex = marker_rect.right - size * 2;
      auto const y = baseline;
      auto const w = std::max(ascent / 6, 2.0f);
      DrawLine(canvas_, stroke_brush, sx, y, ex, y, size);
      DrawLine(canvas_, stroke_brush, sx, y, sx, y - w * 2, size);
      DrawLine(canvas_, stroke_brush, ex, y, ex, y - w * 2, size);
      break;
    }
  }
}

void PaintVisitor::VisitInlineTextBox(InlineTextBox* inline_box) {
  DCHECK(!inline_box->characters().empty());
  const auto& style = inline_box->style();
  const auto& font = inline_box->font();
  auto const text_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(rect_.width(), inline_box->height()));
  FillBackground(canvas_, rect_, *inline_box);
  gfx::Brush text_brush(canvas_, style.color());
  DrawText(canvas_, font, text_brush, text_rect, inline_box->characters());

  if (style.text_decoration_line() != TextDecorationLine::Underline)
    return;

  const auto baseline = text_rect.bottom - inline_box->descent();
  const auto underline = baseline + font.underline();
  auto const size = inline_box->font().underline_thickness();
  const auto& brush = gfx::Brush(canvas_, style.text_decoration_color());
  switch (style.text_decoration_style()) {
    case TextDecorationStyle::Dashed: {
      gfx::StrokeStyle stroke_style;
      stroke_style.set_dash_style(gfx::DashStyle::Dash);
      stroke_style.Realize(canvas_);
      DrawHLine(canvas_, brush, rect_.left, rect_.right, underline, size,
                stroke_style);
      return;
    }

    case TextDecorationStyle::Dotted: {
      gfx::StrokeStyle stroke_style;
      stroke_style.set_dash_style(gfx::DashStyle::Custom);
      // TODO(eval1749): We should understand why |DashStyle::Dot| doesn't
      // show anything. It dashes = {0.0f, 2.0f}.
      stroke_style.set_dashes({1.0f, 2.0f});
      stroke_style.Realize(canvas_);
      DrawHLine(canvas_, brush, rect_.left, rect_.right, underline, size,
                stroke_style);
      return;
    }

    case TextDecorationStyle::Double:
      DrawHLine(canvas_, brush, rect_.left, rect_.right, underline, size * 2);
      return;

    case TextDecorationStyle::Solid:
      DrawHLine(canvas_, brush, rect_.left, rect_.right, underline, size);
      return;

    case TextDecorationStyle::Wavy: {
      const auto descent = inline_box->descent();
      const auto wave_width = size * 3;
      const auto wave_height = size * 1.5;
      // Since "Meiryo" font has large descendant, e.g. 6 with ascend = 14 and
      // underline = 1, we use middle of descend as center line fo wave.
      DrawWave(canvas_, brush, rect_, text_rect.bottom - descent / 2,
               wave_width, wave_height, size);
      return;
    }
  }
  NOTREACHED() << "We should handle text-decoration-style: "
               << static_cast<int>(style.text_decoration_style());
}

void PaintVisitor::VisitInlineUnicodeBox(InlineUnicodeBox* inline_box) {
  auto const text_rect =
      gfx::RectF(gfx::PointF(rect_.left, rect_.top + inline_box->top()),
                 gfx::SizeF(rect_.width(), inline_box->height())) -
      gfx::SizeF(1, 1);
  const auto& font = inline_box->font();
  const auto& style = inline_box->style();
  FillBackground(canvas_, rect_, *inline_box);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, text_rect);
  gfx::Brush text_brush(canvas_, style.color());
  DrawText(canvas_, font, text_brush, text_rect - gfx::SizeF(2, 2),
           inline_box->characters());
  canvas_->DrawRectangle(text_brush, text_rect - gfx::SizeF(1, 1));
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

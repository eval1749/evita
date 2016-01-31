// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <algorithm>

#include "base/logging.h"
#include "evita/gfx/brush.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/controls/text_field_painter.h"

namespace ui {

namespace {

const auto padding_bottom = 1.0f;
const auto padding_left = 5.0f;
const auto padding_right = 5.0f;
const auto padding_top = 1.0f;

gfx::RectF ComputeCaretBounds(const gfx::TextLayout& text_layout,
                              const TextFieldControl::Selection& selection) {
  switch (selection.caret_shape) {
    case TextFieldControl::CaretShape::Bar: {
      auto bar_x = 0.0f;
      auto bar_y = 0.0f;
      const auto is_trailing = false;
      DWRITE_HIT_TEST_METRICS bar_metrics = {0};
      COM_VERIFY(text_layout->HitTestTextPosition(
          static_cast<uint32_t>(selection.focus_offset), is_trailing, &bar_x,
          &bar_y, &bar_metrics));
      if (!bar_metrics.isText)
        return gfx::RectF();
      return gfx::RectF(gfx::PointF(std::round(bar_x), std::round(bar_y)),
                        gfx::SizeF(1.0f, bar_metrics.height));
    }
    case TextFieldControl::CaretShape::Box: {
      auto box_x = 0.0f;
      auto box_y = 0.0f;
      const auto is_trailing = false;
      DWRITE_HIT_TEST_METRICS box_metrics = {0};
      COM_VERIFY(text_layout->HitTestTextPosition(
          static_cast<uint32_t>(selection.focus_offset), is_trailing, &box_x,
          &box_y, &box_metrics));
      if (!box_metrics.isText)
        return gfx::RectF();
      return gfx::RectF(gfx::PointF(box_x, box_y),
                        gfx::SizeF(box_metrics.width, box_metrics.height));
    }
    case TextFieldControl::CaretShape::None:
      return gfx::RectF();
  }
  NOTREACHED();
  return gfx::RectF();
}

gfx::RectF ComputeSelectionBounds(
    const gfx::TextLayout& text_layout,
    const TextFieldControl::Selection& selection) {
  if (selection.collapsed())
    return gfx::RectF();
  uint32_t num_metrics = 0;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  const auto originX = 0.0f;
  const auto originY = 0.0f;
  COM_VERIFY(text_layout->HitTestTextRange(
      static_cast<uint32_t>(selection.start()),
      static_cast<uint32_t>(selection.end() - selection.start()), originX,
      originY, &metrics, 1, &num_metrics));
  DCHECK_EQ(1u, num_metrics);
  return gfx::RectF(gfx::PointF(metrics.left, metrics.top),
                    gfx::SizeF(metrics.width, metrics.height));
}

std::unique_ptr<gfx::TextLayout> CreateTextLayout(
    const base::string16& text,
    const TextFieldControl::Style& style,
    float height) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  const auto kHugeWidth = 1e6f;
  return text_format.CreateLayout(text, gfx::SizeF(kHugeWidth, height));
}

}  // namespace

TextFieldPainter::TextFieldPainter() {}
TextFieldPainter::~TextFieldPainter() {}

void TextFieldPainter::Paint(gfx::Canvas* canvas,
                             const gfx::RectF& bounds,
                             Control::State state,
                             const Control::Style& style,
                             const TextFieldControl::Selection& selection,
                             float scroll_left,
                             const base::string16& text) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->FillRectangle(gfx::Brush(canvas, style.bgcolor), bounds);
  canvas->AddDirtyRect(bounds);
  PaintBorder(canvas, bounds, state, style);
  PaintText(canvas, bounds, state, style, selection, scroll_left, text);
}

void TextFieldPainter::PaintBorder(gfx::Canvas* canvas,
                                   const gfx::RectF& bounds,
                                   Control::State state,
                                   const Control::Style& style) const {
  switch (state) {
    case ui::Control::State::Disabled:
      canvas->FillRectangle(gfx::Brush(canvas, gfx::ColorF(style.shadow, 0.1f)),
                            bounds);
      canvas->DrawRectangle(gfx::Brush(canvas, style.shadow), bounds);
      break;
    case ui::Control::State::Normal:
      canvas->DrawRectangle(gfx::Brush(canvas, style.shadow), bounds);
      break;
    case ui::Control::State::Highlight: {
      gfx::Brush highlight_brush(canvas, gfx::ColorF(style.highlight, 0.5f));
      canvas->DrawRectangle(highlight_brush, bounds - 1.0f, 2.0f);
      break;
    }
    case ui::Control::State::Hovered:
      canvas->DrawRectangle(gfx::Brush(canvas, style.hotlight), bounds);
      break;
  }
}

void TextFieldPainter::PaintText(gfx::Canvas* canvas,
                                 const gfx::RectF& bounds,
                                 Control::State state,
                                 const Control::Style& style,
                                 const TextFieldControl::Selection& selection,
                                 float scroll_left,
                                 const base::string16& text) const {
  DCHECK_EQ(base::string16::npos, text.find('\n'));

  const auto& text_bounds =
      gfx::RectF(gfx::SizeF(bounds.width() - padding_left - padding_right,
                            bounds.height() - padding_top - padding_bottom));
  const auto& viewport =
      gfx::RectF(gfx::PointF(padding_left, padding_top), text_bounds.size());
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, viewport);
  const auto& text_layout = CreateTextLayout(text, style, bounds.height());
  const auto& text_size = text_layout->ComputeSize();
  gfx::Brush text_brush(canvas, state == ui::Control::State::Disabled
                                    ? style.gray_text
                                    : style.color);
  // Draw text at middle of viewport
  const auto& text_origin =
      viewport.origin() +
      gfx::SizeF(scroll_left, (viewport.height() - text_size.height) / 2);
  (*canvas)->DrawTextLayout(text_origin, *text_layout, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);
  if (state != ui::Control::State::Highlight)
    return;
  // Paint selection
  const auto& selection_bounds =
      ComputeSelectionBounds(*text_layout, selection);
  if (!selection_bounds.empty()) {
    const auto& selection_view_bounds = viewport.Intersect(
        gfx::RectF(gfx::ToEnclosingRect(selection_bounds.Offset(text_origin))));
    if (!selection_view_bounds.empty()) {
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.highlight, 0.3f)),
          selection_view_bounds);
    }
  }
  // Paint caret
  if (selection.caret_shape == TextFieldControl::CaretShape::None)
    return;
  const auto& caret_bounds = ComputeCaretBounds(*text_layout, selection);
  if (caret_bounds.empty())
    return;
  const auto& caret_view_bounds = viewport.Intersect(
      gfx::RectF(gfx::ToEnclosingRect(caret_bounds.Offset(text_origin))));
  if (caret_view_bounds.empty())
    return;
  canvas->FillRectangle(gfx::Brush(canvas, gfx::ColorF(style.color)),
                        caret_view_bounds);
}

}  // namespace ui

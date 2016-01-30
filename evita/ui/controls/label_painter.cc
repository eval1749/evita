// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/label_painter.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/direct_write_factory_win.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"

namespace ui {

namespace {
// TODO(eval1749): We should share |CreateTextLayout()| with |ButtonPainter|.
std::unique_ptr<gfx::TextLayout> CreateTextLayout(base::StringPiece16 text,
                                                  const Control::Style& style,
                                                  const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  return text_format.CreateLayout(text.as_string(), size);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LabelPainter
//
LabelPainter::LabelPainter() {}
LabelPainter::~LabelPainter() {}

void LabelPainter::Paint(gfx::Canvas* canvas,
                         const gfx::RectF& bounds,
                         Control::State state,
                         const Control::Style& style,
                         base::StringPiece16 text) const {
  // TODO(eval1749): We should share following code fragment with
  // |ButtonControl|
  // and |TextFieldControl|.
  const auto& text_layout_ = CreateTextLayout(text, style, bounds.size());
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  const auto& text_size = gfx::SizeF(metrics.width, metrics.height);
  // Paint text at middle of control.
  const auto offset = (bounds.size() - text_size) / 2.0f;
  const auto& text_origin_ =
      gfx::PointF(bounds.left, bounds.top + offset.height);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->FillRectangle(gfx::Brush(canvas, style.bgcolor), bounds);
  gfx::Brush text_brush(canvas, style.color);
  (*canvas)->DrawTextLayout(text_origin_, *text_layout_, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);
  canvas->AddDirtyRect(bounds);
}

}  // namespace ui

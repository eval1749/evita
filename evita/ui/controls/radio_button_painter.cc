// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/radio_button_painter.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// RadioButtonPainter
//
RadioButtonPainter::RadioButtonPainter() {}
RadioButtonPainter::~RadioButtonPainter() {}

void RadioButtonPainter::Paint(gfx::Canvas* canvas,
                               const gfx::RectF& bounds,
                               Control::State state,
                               const Control::Style& style,
                               bool checked) const {
  DCHECK(!bounds.empty());

  if (bounds.empty())
    return;

  canvas->FillRectangle(gfx::Brush(canvas, style.bgcolor), bounds);

  auto const size = 12.0f;
  D2D1_ELLIPSE ellipse;
  ellipse.point = bounds.origin() + (bounds.size() / 2);
  ellipse.radiusX = ellipse.radiusY = size / 2;
  gfx::Brush frame_brush(
      canvas, state == Control::State::Hovered ? style.hotlight : style.shadow);
  (*canvas)->DrawEllipse(ellipse, frame_brush);

  if (checked) {
    D2D1_ELLIPSE ellipse2;
    ellipse2.point = ellipse.point;
    ellipse2.radiusX = ellipse2.radiusY = size / 2 - 3;
    gfx::Brush black_brush(canvas, style.color);
    (*canvas)->FillEllipse(ellipse2, black_brush);
  }
  switch (state) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      (*canvas)->FillEllipse(
          ellipse, gfx::Brush(canvas, gfx::ColorF(style.highlight, 0.1f)));
      (*canvas)->DrawEllipse(ellipse, gfx::Brush(canvas, style.highlight));
      break;
    case Control::State::Hovered:
      (*canvas)->FillEllipse(
          ellipse, gfx::Brush(canvas, gfx::ColorF(style.hotlight, 0.1f)));
      (*canvas)->DrawEllipse(ellipse, gfx::Brush(canvas, style.hotlight));
      break;
  }
  canvas->AddDirtyRect(bounds);
}

}  // namespace ui

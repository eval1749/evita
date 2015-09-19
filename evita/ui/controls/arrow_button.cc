// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/arrow_button.h"

#include "evita/gfx/canvas.h"

namespace ui {

ArrowButton::ArrowButton(Direction direction, ButtonListener* listener)
    : Button(listener), direction_(direction) {}

ArrowButton::~ArrowButton() {}

gfx::ColorF ArrowButton::ComputeColor() const {
  switch (state()) {
    case State::Disabled:
      return gfx::ColorF(0, 0, 0, 0.1f);
    case State::Hovered:
      return gfx::ColorF(0, 0, 0, 0.5f);
    case State::Normal:
      return gfx::ColorF(0, 0, 0, 0.3f);
    case State::Pressed:
      return gfx::ColorF(0, 0, 0, 1.0f);
  }
  NOTREACHED();
  return gfx::ColorF(0, 0, 1, 1.0f);
}

void ArrowButton::DrawArrow(gfx::Canvas* canvas) const {
  float factors[4] = {0.0f};
  switch (direction_) {
    case Direction::Down:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = -1.0f;
      break;
    case Direction::Left:
      factors[0] = 1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Right:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = -1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Up:
      factors[0] = -1.0f;
      factors[1] = 1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    default:
      NOTREACHED();
  }

  auto const bounds = GetContentsBounds();
  auto const center_x = bounds.left + bounds.width() / 2;
  auto const center_y = bounds.top + bounds.height() / 2;
  auto const wing_size = bounds.width() / 4;
  auto const pen_width = 2.0f;

  gfx::Brush arrow_brush(canvas, ComputeColor());
  canvas->DrawLine(arrow_brush, gfx::PointF(center_x + factors[0] * wing_size,
                                            center_y + factors[1] * wing_size),
                   gfx::PointF(center_x, center_y), pen_width);
  canvas->DrawLine(arrow_brush, gfx::PointF(center_x + factors[2] * wing_size,
                                            center_y + factors[3] * wing_size),
                   gfx::PointF(center_x, center_y), pen_width);
}

// ui::Widget
void ArrowButton::PaintButton(gfx::Canvas* canvas) {
  auto const bounds = GetContentsBounds();
  canvas->AddDirtyRect(bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->Clear(gfx::ColorF());
  DrawArrow(canvas);
}

}  // namespace ui

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/radio_button_control.h"

#include "evita/gfx_base.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// RadioButtonControl
//
RadioButtonControl::RadioButtonControl(ControlController* controller,
                                       bool checked, const Style& style)
    : Control(controller), checked_(checked), style_(style) {
}

RadioButtonControl::~RadioButtonControl() {
}

void RadioButtonControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  SchedulePaint();
}

void RadioButtonControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;
  checked_ = new_checked;
  SchedulePaint();
}

// ui::Widget
void RadioButtonControl::OnDraw(gfx::Canvas* canvas) {
  if (bounds().empty())
    return;

  auto const bounds = GetContentsBounds();
  canvas->FillRectangle(gfx::Brush(canvas, style_.bgcolor), bounds);

  auto const size = 12.0f;
  D2D1_ELLIPSE ellipse;
  ellipse.point = bounds.origin() + (bounds.size() / 2);
  ellipse.radiusX = ellipse.radiusY = size / 2;
  gfx::Brush frame_brush(canvas, hover() ? style_.hotlight : style_.shadow);
  (*canvas)->DrawEllipse(ellipse, frame_brush);

  if (checked_) {
    D2D1_ELLIPSE ellipse2;
    ellipse2.point = ellipse.point;
    ellipse2.radiusX = ellipse2.radiusY = size / 2 - 3;
    gfx::Brush black_brush(canvas, style_.color);
    (*canvas)->FillEllipse(ellipse2, black_brush);
  }
  switch (state()) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      (*canvas)->FillEllipse(ellipse,
          gfx::Brush(canvas, gfx::ColorF(style_.highlight, 0.1)));
      (*canvas)->DrawEllipse(ellipse,
          gfx::Brush(canvas, style_.highlight));
      break;
    case Control::State::Hover:
      (*canvas)->FillEllipse(ellipse,
          gfx::Brush(canvas, gfx::ColorF(style_.hotlight, 0.1f)));
      (*canvas)->DrawEllipse(ellipse,
          gfx::Brush(canvas, style_.hotlight));
      break;
  }
  canvas->Flush();
  canvas->AddDirtyRect(bounds);
}

}  // namespace ui

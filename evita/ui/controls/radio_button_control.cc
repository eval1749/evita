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
  style_ = new_style;
}

void RadioButtonControl::set_checked(bool new_checked) {
  checked_ = new_checked;
}

// ui::Widget
void RadioButtonControl::OnDraw(gfx::Canvas* gfx) {
  if (bounds().empty())
    return;

  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), bounds());

  auto const size = 12.0f;
  D2D1_ELLIPSE ellipse;
  ellipse.point = gfx::PointF(bounds().left + bounds().width() / 2,
                              bounds().top + bounds().height() / 2);
  ellipse.radiusX = ellipse.radiusY = size / 2;
  gfx::Brush frame_brush(*gfx, hover() ? style_.hotlight : style_.shadow);
  (*gfx)->DrawEllipse(ellipse, frame_brush);

  if (checked_) {
    D2D1_ELLIPSE ellipse2;
    ellipse2.point = ellipse.point;
    ellipse2.radiusX = ellipse2.radiusY = size / 2 - 3;
    gfx::Brush black_brush(*gfx, style_.color);
    (*gfx)->FillEllipse(ellipse2, black_brush);
  }
  switch (state()) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      (*gfx)->FillEllipse(ellipse,
          gfx::Brush(*gfx, gfx::ColorF(style_.highlight, 0.1)));
      (*gfx)->DrawEllipse(ellipse,
          gfx::Brush(*gfx, style_.highlight));
      break;
    case Control::State::Hover:
      (*gfx)->FillEllipse(ellipse,
          gfx::Brush(*gfx, gfx::ColorF(style_.hotlight, 0.1f)));
      (*gfx)->DrawEllipse(ellipse,
          gfx::Brush(*gfx, style_.hotlight));
      break;
  }
  gfx->Flush();
}

}  // namespace ui

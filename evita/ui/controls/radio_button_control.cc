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
void RadioButtonControl::OnDraw(gfx::Graphics* gfx) {
  if (rect().empty())
    return;

  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect());

  auto const size = 12.0f;
  D2D1_ELLIPSE ellipse;
  ellipse.point = gfx::PointF(rect().left + rect().width() / 2,
                              rect().top + rect().height() / 2);
  ellipse.radiusX = ellipse.radiusY = size / 2;
  gfx::Brush frame_brush(*gfx, hover() ? style_.hotlight : style_.shadow);
  (*gfx)->DrawEllipse(ellipse, frame_brush);

  if (checked_) {
    ellipse.radiusX = ellipse.radiusY = size / 2 - 4;
    gfx::Brush black_brush(*gfx, style_.color);
    (*gfx)->FillEllipse(ellipse, black_brush);
  }
  gfx->Flush();
}

}  // namespace ui

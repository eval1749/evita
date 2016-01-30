// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/radio_button_control.h"

#include "evita/ui/controls/radio_button_painter.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// RadioButtonControl
//
RadioButtonControl::RadioButtonControl(ControlController* controller,
                                       bool checked,
                                       const Style& style)
    : Control(controller), checked_(checked), style_(style) {}

RadioButtonControl::~RadioButtonControl() {}

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
  RadioButtonPainter().Paint(canvas, GetContentsBounds(), state(), style_,
                             checked_);
}

}  // namespace ui

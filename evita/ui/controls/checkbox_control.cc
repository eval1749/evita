// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/checkbox_control.h"

#include "evita/ui/controls/checkbox_painter.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
CheckboxControl::CheckboxControl(ControlController* controller,
                                 bool checked,
                                 const Style& style)
    : Control(controller), checked_(checked), style_(style) {}

CheckboxControl::~CheckboxControl() {}

void CheckboxControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  SchedulePaint();
}

void CheckboxControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;
  checked_ = new_checked;
  SchedulePaint();
}

// ui::Widget
void CheckboxControl::OnDraw(gfx::Canvas* canvas) {
  CheckboxPainter().Paint(canvas, GetContentsBounds(), state(), style_,
                          checked_);
}

}  // namespace ui

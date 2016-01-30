// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/button_control.h"

#include "evita/ui/controls/button_painter.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
ButtonControl::ButtonControl(ControlController* controller,
                             const base::string16& text,
                             const Style& style)
    : Control(controller), style_(style), text_(text) {}

ButtonControl::~ButtonControl() {}

void ButtonControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  SchedulePaint();
}

void ButtonControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  SchedulePaint();
}

// ui::Widget
void ButtonControl::DidChangeBounds() {
  Control::DidChangeBounds();
}

void ButtonControl::OnDraw(gfx::Canvas* canvas) {
  ButtonPainter().Paint(canvas, GetContentsBounds(), state(), style_, text_);
}

}  // namespace ui

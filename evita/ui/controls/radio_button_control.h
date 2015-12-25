// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_RADIO_BUTTON_CONTROL_H_
#define EVITA_UI_CONTROLS_RADIO_BUTTON_CONTROL_H_

#include "evita/ui/controls/control.h"

#include "evita/gfx/forward.h"

namespace ui {

class RadioButtonControl final : public Control {
 public:
  RadioButtonControl(ControlController* controller,
                     bool checked,
                     const Style& style);
  ~RadioButtonControl() final;

  const Style& style() const { return style_; }
  void set_style(const Style& style);
  bool checked() const { return checked_; }
  void set_checked(bool checked);

 private:
  // ui::Widget
  void OnDraw(gfx::Canvas* canvas) final;

  bool checked_;
  Style style_;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControl);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_RADIO_BUTTON_CONTROL_H_

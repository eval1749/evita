// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_radio_button_control_h)
#define INCLUDE_evita_ui_controls_radio_button_control_h

#include "evita/ui/controls/control.h"

#include "evita/gfx_base.h"

namespace ui {

class RadioButtonControl final : public Control {
  private: bool checked_;
  private: Style style_;

  public: RadioButtonControl(ControlController* controller,
                             bool checked, const Style& style);
  public: virtual ~RadioButtonControl();

  public: const Style& style() const { return style_; }
  public: void set_style(const Style& style);
  public: bool checked() const { return checked_; }
  public: void set_checked(bool checked);

  // ui::Widget
  private: virtual void OnDraw(gfx::Canvas* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_radio_button_control_h)

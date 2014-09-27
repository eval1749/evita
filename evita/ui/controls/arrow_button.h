// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_arrow_button_h)
#define INCLUDE_evita_ui_controls_arrow_button_h

#include "evita/ui/controls/button.h"

namespace gfx {
class ColorF;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ArrowButton
//
class ArrowButton final : public ui::Button {
  DECLARE_CASTABLE_CLASS(ArrowButton, Button);

  public: enum class Direction {
    Down,
    Left,
    Right,
    Up,
  };

  private: Direction direction_;

  public: ArrowButton(Direction direction, ButtonListener* listener);
  public: virtual ~ArrowButton();

  private: gfx::ColorF ComputeColor() const;
  private: void DrawArrow(gfx::Canvas* canvas) const;

  // ui::Widget
  private: virtual void OnDraw(gfx::Canvas* canvas) override;

  DISALLOW_COPY_AND_ASSIGN(ArrowButton);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_arrow_button_h)

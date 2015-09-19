// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_ARROW_BUTTON_H_
#define EVITA_UI_CONTROLS_ARROW_BUTTON_H_

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

 public:
  enum class Direction {
    Down,
    Left,
    Right,
    Up,
  };

  ArrowButton(Direction direction, ButtonListener* listener);
  ~ArrowButton() final;

 private:
  gfx::ColorF ComputeColor() const;
  void DrawArrow(gfx::Canvas* canvas) const;

  // ui::Button
  void PaintButton(gfx::Canvas* canvas) final;

  Direction direction_;

  DISALLOW_COPY_AND_ASSIGN(ArrowButton);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_ARROW_BUTTON_H_

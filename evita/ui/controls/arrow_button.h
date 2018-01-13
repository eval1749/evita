// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_ARROW_BUTTON_H_
#define EVITA_UI_CONTROLS_ARROW_BUTTON_H_

#include <memory>

#include "evita/ui/base/repeat_controller.h"
#include "evita/ui/controls/button.h"

namespace gfx {
class ColorF;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ArrowButton
//
class ArrowButton : public ui::Button {
  DECLARE_DEPRECATED_CASTABLE_CLASS(ArrowButton, Button);

 public:
  enum class Direction {
    Down,
    Left,
    Right,
    Up,
  };

  ArrowButton(Direction direction, ButtonListener* listener);
  ~ArrowButton() override;

 private:
  gfx::ColorF ComputeColor() const;
  void DidFireRepeatTimer();
  void DrawArrow(gfx::Canvas* canvas) const;

  // ui::Button
  void PaintButton(gfx::Canvas* canvas) final;

  const Direction direction_;

  DISALLOW_COPY_AND_ASSIGN(ArrowButton);
};

//////////////////////////////////////////////////////////////////////
//
// RepetableArrowButton
//
class RepetableArrowButton final : public ArrowButton {
  DECLARE_DEPRECATED_CASTABLE_CLASS(RepetableArrowButton, ArrowButton);

 public:
  RepetableArrowButton(Direction direction, ButtonListener* listener);
  ~RepetableArrowButton() final;

 private:
  void DidFireRepeatTimer();

  // ui::Widget
  void OnMousePressed(const ui::MouseEvent& event) final;
  void OnMouseReleased(const ui::MouseEvent& event) final;

  const std::unique_ptr<RepeatController> repeater_;

  DISALLOW_COPY_AND_ASSIGN(RepetableArrowButton);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_ARROW_BUTTON_H_

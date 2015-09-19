// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_BUTTON_H_
#define EVITA_UI_CONTROLS_BUTTON_H_

#include "evita/ui/widget.h"

namespace ui {

class Button;
class Event;

//////////////////////////////////////////////////////////////////////
//
// ButtonListener
//
class ButtonListener {
 public:
  virtual void DidPressButton(Button* sender, const Event& event) = 0;

 protected:
  ButtonListener();
  virtual ~ButtonListener();
};

//////////////////////////////////////////////////////////////////////
//
// Button
//
class Button : public ui::Widget {
  DECLARE_CASTABLE_CLASS(Button, ui::Widget);

 public:
  enum class State {
    Normal,
    Disabled,
    Hovered,
    Pressed,
  };

  ~Button() override;

  State state() const { return state_; }

 protected:
  explicit Button(ButtonListener* listener);

  virtual void PaintButton(gfx::Canvas* canvas);
  void SetState(State new_state);

  // ui::Widget
  void DidChangeBounds() override;
  void DidShow() override;
  void OnDraw(gfx::Canvas* canvas) override;
  void OnMouseEntered(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  void OnMousePressed(const ui::MouseEvent& event) override;
  void OnMouseReleased(const ui::MouseEvent& event) override;

 private:
  bool IsDirty(const gfx::Canvas* canvas) const;
  void MarkDirty();

  int canvas_bitmap_id_;
  bool dirty_;
  ButtonListener* listener_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Button);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_BUTTON_H_

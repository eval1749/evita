// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_CONTROL_H_
#define EVITA_UI_CONTROLS_CONTROL_H_

#include "evita/gfx/color_f.h"
#include "evita/ui/widget.h"

namespace ui {

class ControlController;
class TextInputDelegate;

class Control : public Widget {
  DECLARE_CASTABLE_CLASS(Control, Widget);

 public:
  enum class State {
    Normal,
    Disabled,
    Highlight,
    Hovered,
  };

  struct Style final {
    gfx::ColorF bgcolor;
    gfx::ColorF color;
    base::string16 font_family;
    float font_size;
    gfx::ColorF gray_text;
    gfx::ColorF highlight;
    gfx::ColorF hotlight;
    gfx::ColorF shadow;

    bool operator==(const Style& other) const;
    bool operator!=(const Style& other) const;
  };

  ~Control() override;

  bool disabled() const { return state_ == State::Disabled; }
  void set_disabled(bool new_disabled);
  virtual bool focusable() const;
  ControlController* controller() const { return controller_; }
  void set_text_input_delegate(TextInputDelegate* delegate);

 protected:
  explicit Control(ControlController* controller);

  bool hover() const { return state_ == State::Hovered; }
  State state() const { return state_; }

  virtual void DidChangeState();

  // ui::EventTarget
  void OnKeyEvent(ui::KeyEvent* event) override;
  void OnMouseEvent(ui::MouseEvent* event) override;

  // ui::Widget
  void DidKillFocus(ui::Widget* focused_window) override;
  void DidRealize() override;
  void DidSetFocus(ui::Widget* last_focused) override;
  void WillDestroyWidget() override;

 private:
  ControlController* controller_;
  TextInputDelegate* text_input_delegate_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_CONTROL_H_

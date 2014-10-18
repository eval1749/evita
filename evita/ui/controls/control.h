// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_control_h)
#define INCLUDE_evita_ui_controls_control_h

#include "evita/gfx/color_f.h"
#include "evita/ui/widget.h"

namespace ui {

class ControlController;
class TextInputDelegate;

class Control : public Widget {
  DECLARE_CASTABLE_CLASS(Control, Widget);

  public: enum class State {
    Normal,
    Disabled,
    Highlight,
    Hovered,
  };
  public: struct Style {
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

  private: ControlController* controller_;
  private: TextInputDelegate* text_input_delegate_;
  private: State state_;

  public: explicit Control(ControlController* controller);
  public: virtual ~Control();

  public: bool disabled() const { return state_ == State::Disabled; }
  public: void set_disabled(bool new_disabled);
  public: virtual bool focusable() const;
  public: ControlController* controller() const { return controller_; }
  protected: bool hover() const { return state_ == State::Hovered; }
  protected: State state() const { return state_; }
  public: void set_text_input_delegate(TextInputDelegate* delegate);

  protected: virtual void DidChangeState();

  // ui::EventTarget
  protected: virtual void OnKeyEvent(ui::KeyEvent* event) override;
  protected: virtual void OnMouseEvent(ui::MouseEvent* event) override;

  // ui::Widget
  protected: virtual void DidKillFocus(ui::Widget* focused_window) override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidSetFocus(ui::Widget* last_focused) override;
  protected: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_control_h)

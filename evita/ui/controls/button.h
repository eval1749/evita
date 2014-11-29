// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_button_h)
#define INCLUDE_evita_ui_controls_button_h

#include "evita/ui/widget.h"

namespace ui {

class Button;
class Event;

//////////////////////////////////////////////////////////////////////
//
// ButtonListener
//
class ButtonListener {
  protected: ButtonListener();
  protected: virtual ~ButtonListener();

  public: virtual void DidPressButton(Button* sender, const Event& event) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// Button
//
class Button : public ui::Widget {
  DECLARE_CASTABLE_CLASS(Button, ui::Widget);

  public: enum class State {
    Normal,
    Disabled,
    Hovered,
    Pressed,
  };

  private: int canvas_bitmap_id_;
  private: bool dirty_;
  private: ButtonListener* listener_;
  private: State state_;

  protected: Button(ButtonListener* listener);
  public: virtual ~Button();

  public: State state() const { return state_; }

  private: bool IsDirty(const gfx::Canvas* canvas) const;
  private: void MarkDirty();
  protected: virtual void PaintButton(gfx::Canvas* canvas);
  protected: void SetState(State new_state);

  // ui::Widget
  protected: virtual void DidChangeBounds() override;
  protected: virtual void DidShow() override;
  protected: void OnDraw(gfx::Canvas* canvas) override;
  protected: void OnMouseEntered(const ui::MouseEvent& event) override;
  protected: void OnMouseExited(const ui::MouseEvent& event) override;
  protected: void OnMousePressed(const ui::MouseEvent& event) override;
  protected: void OnMouseReleased(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(Button);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_button_h)

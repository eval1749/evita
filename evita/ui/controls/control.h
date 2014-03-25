// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_control_h)
#define INCLUDE_evita_ui_controls_control_h

#include "evita/ui/widget.h"
#include "evita/gfx/color_f.h"

namespace ui {

class ControlController;

class Control : public ui::Widget {
  public: struct Style {
    gfx::ColorF bgcolor;
    gfx::ColorF color;
    gfx::ColorF highlight;
    gfx::ColorF hotlight;
    gfx::ColorF shadow;
  };

  private: ControlController* controller_;
  private: bool hover_;

  public: Control(ControlController* controller);
  public: virtual ~Control();

  public: virtual bool focusable() const;
  public: ControlController* controller() const { return controller_; }
  protected: bool hover() const { return hover_; }

  // ui::Widget
  protected: virtual void OnKeyPressed(const KeyboardEvent& event) override;
  protected: virtual void OnKeyReleased(const KeyboardEvent& event) override;
  protected: virtual void OnMouseExited(const MouseEvent& event) override;
  protected: virtual void OnMouseMoved(const MouseEvent& event) override;
  protected: virtual void OnMousePressed(const MouseEvent& event) override;
  protected: virtual void OnMouseReleased(const MouseEvent& event) override;
  protected: virtual void OnMouseWheel(const MouseWheelEvent& event) override;
  protected: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_control_h)

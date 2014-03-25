// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_control_h)
#define INCLUDE_evita_ui_controls_control_h

#include "evita/ui/widget.h"

namespace ui {

class ControlController;

class Control : public ui::Widget {
  private: ControlController* controller_;

  public: Control(ControlController* controller);
  public: virtual ~Control();

  public: ControlController* controller() const { return controller_; }

  // ui::Widget
  private: virtual void OnKeyPressed(const KeyboardEvent& event) override;
  private: virtual void OnKeyReleased(const KeyboardEvent& event) override;
  private: virtual void OnMouseMoved(const MouseEvent& event) override;
  private: virtual void OnMousePressed(const MouseEvent& event) override;
  private: virtual void OnMouseReleased(const MouseEvent& event) override;
  private: virtual void OnMouseWheel(const MouseWheelEvent& event) override;
  private: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_control_h)

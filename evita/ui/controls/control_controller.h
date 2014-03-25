// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_control_controller_h)
#define INCLUDE_evita_ui_controls_control_controller_h

#include "base/basictypes.h"

namespace ui {

class Control;
class KeyboardEvent;
class MouseEvent;
class MouseWheelEvent;

class ControlController {
  protected: ControlController();
  public: virtual ~ControlController();

  public: virtual void OnKeyPressed(Control* control,
                                    const KeyboardEvent& event);
  public: virtual void OnKeyReleased(Control* control,
                                     const KeyboardEvent& event);
  public: virtual void OnMouseMoved(Control* control,
                                    const MouseEvent& event);
  public: virtual void OnMousePressed(Control* control,
                                      const MouseEvent& event);
  public: virtual void OnMouseReleased(Control* control,
                                       const MouseEvent& event);
  public: virtual void OnMouseWheel(Control* control,
                                    const MouseWheelEvent& event);
  public: virtual void WillDestroyControl(Control* control);

  DISALLOW_COPY_AND_ASSIGN(ControlController);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_control_controller_h)

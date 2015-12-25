// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_CONTROL_CONTROLLER_H_
#define EVITA_UI_CONTROLS_CONTROL_CONTROLLER_H_

#include "base/macros.h"

namespace ui {

class Control;
class KeyEvent;
class MouseEvent;
class Widget;

class ControlController {
 public:
  virtual ~ControlController();

  virtual void DidKillFocus(Control* control, Widget* focused_widget);
  virtual void DidRealize(Control* control);
  virtual void DidSetFocus(Control* control, Widget* last_focused_widget);
  virtual void OnKeyEvent(Control* control, const KeyEvent& event);
  virtual void OnMouseEvent(Control* control, const MouseEvent& event);
  virtual void WillDestroyControl(Control* control);

 protected:
  ControlController();

 private:
  DISALLOW_COPY_AND_ASSIGN(ControlController);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_CONTROL_CONTROLLER_H_

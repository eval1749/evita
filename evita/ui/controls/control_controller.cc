// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control_controller.h"

#include "base/logging.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ControlController
//
ControlController::ControlController() {}

ControlController::~ControlController() {}

void ControlController::DidKillFocus(Control* control, Widget* widget) {
  DCHECK(control);
}

void ControlController::DidRealize(Control* control) {
  DCHECK(control);
}

void ControlController::DidSetFocus(Control* control, Widget* widget) {
  DCHECK(control);
}

void ControlController::OnKeyEvent(Control* control, const KeyEvent& event) {
  DCHECK(control);
}

void ControlController::OnMouseEvent(Control* control,
                                     const MouseEvent& event) {
  DCHECK(control);
}

void ControlController::WillDestroyControl(Control* control) {
  DCHECK(control);
}

}  // namespace ui

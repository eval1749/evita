// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control_controller.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ControlController
//
ControlController::ControlController() {
}

ControlController::~ControlController() {
}

void ControlController::DidKillFocus(Control*, Widget*) {
}

void ControlController::DidRealize(Control*) {
}

void ControlController::DidSetFocus(Control*, Widget*) {
}

void ControlController::OnKeyEvent(Control*, const KeyEvent&) {
}

void ControlController::OnMouseEvent(Control*, const MouseEvent&) {
}

void ControlController::WillDestroyControl(Control*) {
}

}  // namespace ui

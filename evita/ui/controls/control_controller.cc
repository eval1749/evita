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

void ControlController::OnKeyPressed(Control*, const KeyboardEvent&) {
}

void ControlController::OnKeyReleased(Control*, const KeyboardEvent&) {
}

void ControlController::OnMouseMoved(Control*, const MouseEvent&) {
}

void ControlController::OnMousePressed(Control*, const MouseEvent&) {
}

void ControlController::OnMouseReleased(Control*, const MouseEvent&) {
}

void ControlController::OnMouseWheel(Control*, const MouseWheelEvent&) {
}

void ControlController::WillDestroyControl(Control*) {
}

}  // namespace ui

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_control_controller.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormControlController
//
FormControlController::FormControlController(
    domapi::EventTargetId event_target_id)
    : EventSource(event_target_id) {
}

FormControlController::~FormControlController() {
}

void FormControlController::OnKeyPressed(ui::Control*,
                                         const ui::KeyboardEvent& event) {
  DispatchKeyboardEvent(event);
}

void FormControlController::OnKeyReleased(ui::Control*,
                                          const ui::KeyboardEvent& event) {
  DispatchKeyboardEvent(event);
}

void FormControlController::OnMouseMoved(ui::Control*,
                                         const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMousePressed(ui::Control*,
                                           const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMouseReleased(ui::Control*,
                                            const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMouseWheel(ui::Control*,
                                         const ui::MouseWheelEvent& event) {
  DispatchWheelEvent(event);
}

void FormControlController::WillDestroyControl(ui::Control*) {
  delete this;
}

}  // namespace views

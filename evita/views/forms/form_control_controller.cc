// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_control_controller.h"

#include "common/memory/singleton.h"
#include "evita/dom/public/view_event.h"
#include "evita/ui/controls/control.h"
#include "evita/views/forms/form_control_set.h"

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

// ui::ControlController
void FormControlController::DidKillFocus(ui::Control*,
                                         ui::Widget* related_widget) {
  DispatchFocusEvent(domapi::EventType::Blur,
      FormControlSet::instance()->MaybeControlId(related_widget));
}

void FormControlController::DidRealize(ui::Control* control) {
  FormControlSet::instance()->Register(control, event_target_id());
}

void FormControlController::DidSetFocus(ui::Control*,
                                        ui::Widget* related_widget) {
  DispatchFocusEvent(domapi::EventType::Focus,
    FormControlSet::instance()->MaybeControlId(related_widget));
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

void FormControlController::WillDestroyControl(ui::Control* control) {
  FormControlSet::instance()->Unregister(control);
  delete this;
}

}  // namespace views

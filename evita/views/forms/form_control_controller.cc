// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_control_controller.h"

#include "common/memory/singleton.h"
#include "evita/dom/public/view_event.h"
#include "evita/ui/base/ime/text_composition.h"
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

void FormControlController::OnKeyEvent(ui::Control*,
                                       const ui::KeyEvent& event) {
  DispatchKeyboardEvent(event);
}

void FormControlController::OnMouseEvent(ui::Control*,
                                         const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::WillDestroyControl(ui::Control* control) {
  FormControlSet::instance()->Unregister(control);
  delete this;
}

// ui::TextInputDelegate
void FormControlController::DidCommitComposition(
    const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionCommit,
                               composition);
}

void FormControlController::DidFinishComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionEnd,
                               ui::TextComposition());
}
void FormControlController::DidStartComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionStart,
                               ui::TextComposition());
}
void FormControlController::DidUpdateComposition(
    const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionUpdate,
                               composition);
}

ui::Widget* FormControlController::GetClientWindow() {
  auto const control = FormControlSet::instance()->MaybeControl(
      event_target_id());
  DCHECK(control);
  return control;
}

}  // namespace views

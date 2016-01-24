// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/checkbox_control.h"

#include "evita/dom/events/form_event.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
CheckboxControl::CheckboxControl() : checked_(false) {}

CheckboxControl::~CheckboxControl() {}

void CheckboxControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;
  // TODO(eval1749): Dispatch |beforeinput| and |input| event.
  checked_ = new_checked;
  DispatchChangeEvent();
}

bool CheckboxControl::DispatchEvent(Event* event,
                                    ExceptionState* exception_state) {
  CR_DEFINE_STATIC_LOCAL(base::string16, kChangeEvent, (L"change"));

  if (auto const form_event = event->as<FormEvent>()) {
    HandlingFormEventScope scope(this);
    if (event->type() == kChangeEvent) {
      auto const new_checked = !form_event->data().empty();
      set_checked(new_checked);
    }
    return false;
  }

  return FormControl::DispatchEvent(event, exception_state);
}

}  // namespace dom

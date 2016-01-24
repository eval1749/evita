// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/radio_button_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"

namespace dom {

namespace {
RadioButtonControl* FindCheckedRadioButton(const Form* form,
                                           const base::string16& name) {
  if (!form)
    return nullptr;
  for (auto control : form->controls()) {
    auto const radio_button = control->as<RadioButtonControl>();
    if (radio_button && radio_button->name() == name &&
        radio_button->checked()) {
      return radio_button;
    }
  }
  return nullptr;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RadioButtonControl
//
RadioButtonControl::RadioButtonControl(const base::string16& name)
    : ScriptableBase(name), checked_(false) {}

RadioButtonControl::~RadioButtonControl() {}

void RadioButtonControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;

  if (new_checked) {
    if (auto const radio_button = FindCheckedRadioButton(form(), name())) {
      if (handling_form_event()) {
        HandlingFormEventScope scope(radio_button);
        radio_button->set_checked(false);
      } else {
        radio_button->set_checked(false);
      }
    }
  }

  // TODO(eval1749): Dispatch |beforeinput| and |input| event.
  checked_ = new_checked;
  DispatchChangeEvent();
}

// EventTarget
bool RadioButtonControl::DispatchEvent(Event* event,
                                       ExceptionState* exception_state) {
  CR_DEFINE_STATIC_LOCAL(base::string16, kChangeEvent, (L"change"));

  if (auto const form_event = event->as<FormEvent>()) {
    HandlingFormEventScope scope(this);
    if (event->type() == kChangeEvent)
      set_checked(true);
    return false;
  }

  return FormControl::DispatchEvent(event, exception_state);
}

}  // namespace dom

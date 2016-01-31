// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/radio_button_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"

namespace dom {

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
  checked_ = new_checked;
}

// FormControl
std::unique_ptr<domapi::FormControl> RadioButtonControl::Paint(
    const FormPaintInfo& paint_info) const {
  auto state = ComputeState(paint_info);
  if (checked_)
    state.set_checked();
  return std::make_unique<domapi::RadioButton>(event_target_id(), bounds(),
                                               state);
}

}  // namespace dom

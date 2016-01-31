// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/checkbox_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/public/form_controls.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
CheckboxControl::CheckboxControl() {}
CheckboxControl::~CheckboxControl() {}

void CheckboxControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;
  checked_ = new_checked;
  NotifyControlChange();
}

// FormControl
std::unique_ptr<domapi::FormControl> CheckboxControl::Paint(
    const FormPaintInfo& paint_info) const {
  auto state = ComputeState(paint_info);
  if (checked_)
    state.set_checked();
  return std::make_unique<domapi::Checkbox>(event_target_id(), bounds(), state);
}

}  // namespace dom

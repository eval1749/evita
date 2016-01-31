// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/button_control.h"

#include "evita/dom/forms/form_control.h"
#include "evita/dom/public/form_controls.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
ButtonControl::ButtonControl(const base::string16& text) : text_(text) {}

ButtonControl::~ButtonControl() {}

void ButtonControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  NotifyControlChange();
}

// FormControl
std::unique_ptr<domapi::FormControl> ButtonControl::Paint(
    const FormPaintInfo& paint_info) const {
  auto state = ComputeState(paint_info);
  return std::make_unique<domapi::Button>(event_target_id(), bounds(), state,
                                          text_);
}

}  // namespace dom

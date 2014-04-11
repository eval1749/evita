// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_control_set.h"

#include "evita/ui/controls/control.h"

namespace views {

FormControlSet::FormControlSet() {
}

FormControlSet::~FormControlSet() {
}

ui::Control* FormControlSet::MaybeControl(
    domapi::EventTargetId event_target_id) const {
  auto const it = id_map_.find(event_target_id);
  return it == id_map_.end() ? nullptr : it->second;
}

domapi::EventTargetId FormControlSet::MaybeControlId(
    ui::Widget* widget) const {
  if (!widget)
    return domapi::kInvalidEventTargetId;
  auto const control = widget->as<ui::Control>();
  if (!control)
    return domapi::kInvalidEventTargetId;
  auto const it = control_map_.find(control);
  return it == control_map_.end() ? domapi::kInvalidEventTargetId : it->second;
}

void FormControlSet::Register(ui::Control* control,
                        domapi::EventTargetId target_id) {
  control_map_[control] = target_id;
  id_map_[target_id] = control;
}

void FormControlSet::Unregister(ui::Control* control) {
  auto const control_it = control_map_.find(control);
  if (control_it == control_map_.end())
    return;
  auto const event_target_id = control_it->second;
  control_map_.erase(control_it);
  auto id_it = id_map_.find(event_target_id);
  if (id_it == id_map_.end())
    return;
  id_map_.erase(id_it);
}

}  // namespace views

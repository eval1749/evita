// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_paint_state.h"

#include "base/logging.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_controls.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormPaintState
//
FormPaintState::FormPaintState() {}
FormPaintState::~FormPaintState() {}

bool FormPaintState::IsChanged(const domapi::FormControl& control) const {
  const auto& it = control_map_.find(control.id());
  if (it == control_map_.end())
    return true;
  return control != *it->second;
}

void FormPaintState::Update(std::unique_ptr<domapi::Form> form) {
  control_map_.clear();
  for (const auto& control : form->controls()) {
    const auto& result = control_map_.emplace(control->id(), control);
    DCHECK(result.second) << "Duplicate control id " << control->id();
  }
  form_ = std::move(form);
}

}  // namespace views

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/form.h"

#include "evita/dom/public/form_controls.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// Form
//
Form::Form(int id) : id_(id) {}

Form::~Form() {
  for (const auto& control : controls_)
    delete control;
}

bool Form::operator==(const Form& other) const {
  if (this == &other)
    return true;
  if (bounds_ != other.bounds_)
    return false;
  if (controls_.size() != other.controls_.size())
    return false;
  auto other_controls = other.controls_.begin();
  for (const auto& control : other.controls_) {
    if (control != *other_controls)
      return false;
  }
  return true;
}

}  // namespace domapi

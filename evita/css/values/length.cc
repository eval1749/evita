// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/values/length.h"

#include "base/logging.h"
#include "evita/css/values/unit.h"

namespace css {

Length::Length(float value) : Length(value, Unit::px) {}

Length::Length(float number, Unit unit) : Dimension(number, unit) {
  DCHECK(is_length()) << *this;
}

Length::Length() : Length(0.0f, Unit::px) {}
Length::~Length() {}

float Length::value() const {
  DCHECK(is_absolute_length()) << *this;
  return number();
}

}  // namespace css

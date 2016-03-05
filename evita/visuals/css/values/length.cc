// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/values/length.h"

#include "base/logging.h"
#include "evita/visuals/css/values/unit.h"

namespace visuals {
namespace css {

Length::Length(float value) : Dimension(value, Unit::px) {}
Length::Length() : Dimension(0.0f, Unit::px) {}
Length::~Length() {}

float Length::value() const {
  DCHECK(is_absolute_length()) << *this;
  return number();
}

}  // namespace css
}  // namespace visuals

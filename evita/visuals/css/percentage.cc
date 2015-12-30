// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/percentage.h"

namespace visuals {
namespace css {

Percentage::Percentage(float value) : value_(value) {}
Percentage::Percentage(const Percentage& other) : value_(other.value_) {}
Percentage::Percentage() {}
Percentage::~Percentage() {}

bool Percentage::operator==(const Percentage& other) const {
  return value_ == other.value_;
}

bool Percentage::operator!=(const Percentage& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Percentage& percentage) {
  return ostream << percentage.value() << '%';
}

}  // namespace css
}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/length.h"

namespace visuals {
namespace css {

Length::Length(float value) : value_(value) {}
Length::Length(const Length& other) : value_(other.value_) {}
Length::Length() {}
Length::~Length() {}

bool Length::operator==(const Length& other) const {
  return value_ == other.value_;
}

bool Length::operator!=(const Length& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Length& length) {
  return ostream << length.value();
}

}  // namespace css
}  // namespace visuals

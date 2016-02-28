// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/dimension.h"

#include "evita/visuals/css/unit.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Dimension
//
Dimension::Dimension(float number, Unit unit) : number_(number), unit_(unit) {}

Dimension::Dimension(const Dimension& other)
    : Dimension(other.number_, other.unit_) {}

Dimension::~Dimension() {}

Dimension& Dimension::operator=(const Dimension& other) {
  number_ = other.number_;
  unit_ = other.unit_;
  return *this;
}

bool Dimension::operator==(const Dimension& other) const {
  return number_ == other.number_ && unit_ == other.unit_;
}

bool Dimension::operator!=(const Dimension& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Dimension& dimension) {
  return ostream << dimension.number() << dimension.unit();
}

}  // namespace css
}  // namespace visuals

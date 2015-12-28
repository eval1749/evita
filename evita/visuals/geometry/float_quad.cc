// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_quad.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatQuad
//
FloatQuad::FloatQuad(const std::array<FloatPoint, 4>& data) : data_(data) {}

FloatQuad::FloatQuad(const FloatQuad& other) : FloatQuad(other.data_) {}

FloatQuad::FloatQuad() {}
FloatQuad::~FloatQuad() {}

FloatQuad& FloatQuad::operator=(const FloatQuad& other) {
  data_ = other.data_;
  return *this;
}

bool FloatQuad::operator==(const FloatQuad& other) const {
  if (this == &other)
    return true;
  return data_ == other.data_;
}

bool FloatQuad::operator!=(const FloatQuad& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const FloatQuad& quad) {
  return ostream << '[' << quad.data()[0] << ', ' << quad.data()[1] << ', '
                 << quad.data()[2] << ', ' << quad.data()[3] << ']';
}

}  // namespace visuals

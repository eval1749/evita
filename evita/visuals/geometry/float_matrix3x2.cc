// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_matrix3x2.h"

#include "evita/visuals/geometry/float_point.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatMatrix3x2
//
FloatMatrix3x2::FloatMatrix3x2(const std::array<float, 6>& data)
    : data_(data) {}

FloatMatrix3x2::FloatMatrix3x2(const FloatMatrix3x2& other)
    : FloatMatrix3x2(other.data_) {}

FloatMatrix3x2::FloatMatrix3x2() {}
FloatMatrix3x2::~FloatMatrix3x2() {}

FloatMatrix3x2& FloatMatrix3x2::operator=(const FloatMatrix3x2& other) {
  data_ = other.data_;
  return *this;
}

bool FloatMatrix3x2::operator==(const FloatMatrix3x2& other) const {
  if (this == &other)
    return true;
  return data_ == other.data_;
}

bool FloatMatrix3x2::operator!=(const FloatMatrix3x2& other) const {
  return !operator==(other);
}

FloatMatrix3x2 FloatMatrix3x2::operator*(const FloatMatrix3x2& other) const {
  return FloatMatrix3x2(
      {data_[0] * other.data_[0] + data_[1] * other.data_[2],
       data_[1] * other.data_[1] + data_[1] * other.data_[3],
       data_[2] * other.data_[0] + data_[3] * other.data_[2],
       data_[3] * other.data_[1] + data_[3] * other.data_[3],
       data_[4] * other.data_[0] + data_[5] * other.data_[2] + other.data_[4],
       data_[5] * other.data_[1] + data_[5] * other.data_[3] + other.data_[5]});
}

// static
FloatMatrix3x2 FloatMatrix3x2::Identity() {
  return FloatMatrix3x2({1, 0, 0, 1, 0, 0});
}

// static
FloatMatrix3x2 FloatMatrix3x2::Scale(float sx,
                                     float sy,
                                     const FloatPoint& center) {
  return FloatMatrix3x2({sx, 0, 0, sy, center.x() - sx * center.x(),
                         center.y() - sy * center.y()});
}

// static
FloatMatrix3x2 FloatMatrix3x2::Translation(float x, float y) {
  return FloatMatrix3x2({1, 0, 0, 1, x, y});
}

std::ostream& operator<<(std::ostream& ostream, const FloatMatrix3x2& matrix) {
  return ostream << '[' << matrix.data()[0] << ', ' << matrix.data()[1] << ', '
                 << matrix.data()[2] << ', ' << matrix.data()[3] << ', '
                 << matrix.data()[4] << ', ' << matrix.data()[5] << ']';
}

}  // namespace visuals

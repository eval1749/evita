// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/geometry/float_quad.h"

namespace visuals {

namespace {
float min4(float a, float b, float c, float d) {
  return std::min(std::min(a, b), std::min(c, d));
}

float max4(float a, float b, float c, float d) {
  return std::max(std::max(a, b), std::max(c, d));
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FloatQuad
//
FloatQuad::FloatQuad(const FloatPoint& point1,
                     const FloatPoint& point2,
                     const FloatPoint& point3,
                     const FloatPoint& point4)
    : data_({point1, point2, point3, point4}) {}

FloatQuad::FloatQuad(const std::array<FloatPoint, 4>& data) : data_(data) {}

FloatQuad::FloatQuad(const FloatRect& rect)
    : FloatQuad(rect.origin(),
                rect.top_right(),
                rect.bottom_right(),
                rect.bottom_left()) {}

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

FloatRect FloatQuad::ComputeBoundingBox() const {
  const auto left =
      min4(data_[0].x(), data_[1].x(), data_[2].x(), data_[3].x());
  const auto top = min4(data_[0].y(), data_[1].y(), data_[2].y(), data_[3].y());
  const auto right =
      max4(data_[0].x(), data_[1].x(), data_[2].x(), data_[3].x());
  const auto bottom =
      max4(data_[0].y(), data_[1].y(), data_[2].y(), data_[3].y());
  if (left == right || top == bottom)
    return FloatRect();
  return FloatRect(FloatPoint(left, top),
                   FloatSize(right - left, bottom - top));
}

std::ostream& operator<<(std::ostream& ostream, const FloatQuad& quad) {
  return ostream << '[' << quad.data()[0] << ', ' << quad.data()[1] << ', '
                 << quad.data()[2] << ', ' << quad.data()[3] << ']';
}

}  // namespace visuals

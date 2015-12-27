// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/style/thickness.h"

#include "evita/visuals/geometry/float_size.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Thickness
//
Thickness::Thickness(float top, float left, float bottom, float right)
    : bottom_(bottom), left_(left), right_(right), top_(top) {}

Thickness::Thickness(const Thickness& other)
    : Thickness(other.top_, other.left_, other.bottom_, other.right_) {}

Thickness::Thickness() {}
Thickness::~Thickness() {}

bool Thickness::operator==(const Thickness& other) const {
  return top_ == other.top_ && left_ == other.left_ &&
         bottom_ == other.bottom_ && right_ == other.right_;
}

bool Thickness::operator!=(const Thickness& other) const {
  return !operator==(other);
}

FloatSize Thickness::bottom_right() const {
  return FloatSize(right_, bottom_);
}

FloatSize Thickness::top_left() const {
  return FloatSize(left_, top_);
}

bool Thickness::HasValue() const {
  return bottom_ != 0 || left_ != 0 || right_ != 0 || top_ != 0;
}

std::ostream& operator<<(std::ostream& ostream, const Thickness& thickness) {
  ostream << "Thickness(";
  auto delimiter = "";
  if (thickness.top() != 0.0f) {
    ostream << delimiter << "top=" << thickness.top();
    delimiter = ", ";
  }
  if (thickness.left() != 0.0f) {
    ostream << delimiter << "left=" << thickness.left();
    delimiter = ", ";
  }
  if (thickness.bottom() != 0.0f) {
    ostream << delimiter << "bottom=" << thickness.bottom();
    delimiter = ", ";
  }
  if (thickness.right() != 0.0f) {
    ostream << delimiter << "right=" << thickness.right();
    delimiter = ", ";
  }
  return ostream << ')';
}

}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/style/padding.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Padding
//
Padding::Padding(float left, float top, float right, float bottom)
    : thickness_(top, left, bottom, right) {}

Padding::Padding(const Padding& other) : thickness_(other.thickness_) {}
Padding::Padding() {}
Padding::~Padding() {}

bool Padding::operator==(const Padding& other) const {
  return thickness_ == other.thickness_;
}

bool Padding::operator!=(const Padding& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Padding& padding) {
  ostream << "Padding(";
  auto delimiter = "";
  if (padding.top() != 0.0f) {
    ostream << delimiter << "top=" << padding.top();
    delimiter = ", ";
  }
  if (padding.left() != 0.0f) {
    ostream << delimiter << "left=" << padding.left();
    delimiter = ", ";
  }
  if (padding.bottom() != 0.0f) {
    ostream << delimiter << "bottom=" << padding.bottom();
    delimiter = ", ";
  }
  if (padding.right() != 0.0f) {
    ostream << delimiter << "right=" << padding.right();
    delimiter = ", ";
  }
  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

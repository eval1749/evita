// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/style/border.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Border
//
Border::Border(float top, float left, float bottom, float right)
    : thickness_(top, left, bottom, right) {}

Border::Border(const Border& other) : thickness_(other.thickness_) {}
Border::Border() {}
Border::~Border() {}

bool Border::operator==(const Border& other) const {
  return thickness_ == other.thickness_;
}

bool Border::operator!=(const Border& other) const {
  return !operator==(other);
}

bool Border::HasValue() const {
  return thickness_.HasValue();
}

std::ostream& operator<<(std::ostream& ostream, const Border& border) {
  ostream << "Border(";
  auto delimiter = "";
  if (border.top() != 0.0f) {
    ostream << delimiter << "top=" << border.top();
    delimiter = ", ";
  }
  if (border.left() != 0.0f) {
    ostream << delimiter << "left=" << border.left();
    delimiter = ", ";
  }
  if (border.bottom() != 0.0f) {
    ostream << delimiter << "bottom=" << border.bottom();
    delimiter = ", ";
  }
  if (border.right() != 0.0f) {
    ostream << delimiter << "right=" << border.right();
    delimiter = ", ";
  }
  return ostream << ')';
}

}  // namespace visuals

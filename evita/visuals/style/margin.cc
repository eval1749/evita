// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/style/margin.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Margin
//
Margin::Margin(float top, float left, float bottom, float right)
    : thickness_(top, left, bottom, right) {}

Margin::Margin(const Margin& other) : thickness_(other.thickness_) {}
Margin::Margin() {}
Margin::~Margin() {}

bool Margin::operator==(const Margin& other) const {
  return thickness_ == other.thickness_;
}

bool Margin::operator!=(const Margin& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Margin& margin) {
  ostream << "Margin(";
  auto delimiter = "";
  if (margin.top() != 0.0f) {
    ostream << delimiter << "top=" << margin.top();
    delimiter = ", ";
  }
  if (margin.left() != 0.0f) {
    ostream << delimiter << "left=" << margin.left();
    delimiter = ", ";
  }
  if (margin.bottom() != 0.0f) {
    ostream << delimiter << "bottom=" << margin.bottom();
    delimiter = ", ";
  }
  if (margin.right() != 0.0f) {
    ostream << delimiter << "right=" << margin.right();
    delimiter = ", ";
  }
  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/layout/margin.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Margin
//
Margin::Margin(float top, float right, float bottom, float left)
    : thickness_(top, right, bottom, left) {}

Margin::Margin(float top, float right_left, float bottom)
    : Margin(top, right_left, bottom, right_left) {}

Margin::Margin(float top_bottom, float right_left)
    : Margin(top_bottom, right_left, top_bottom, right_left) {}

Margin::Margin(float width) : Margin(width, width, width, width) {}

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

}  // namespace visuals

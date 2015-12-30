// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/border.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Border
//
Border::Border(const Color& color,
               float top,
               float left,
               float bottom,
               float right)
    : bottom_color_(color),
      left_color_(color),
      right_color_(color),
      top_color_(color),
      thickness_(top, left, bottom, right) {}

Border::Border(const Color& color, float width)
    : Border(color, width, width, width, width) {}

Border::Border(const Border& other)
    : bottom_color_(other.bottom_color_),
      left_color_(other.left_color_),
      right_color_(other.right_color_),
      top_color_(other.top_color_),
      thickness_(other.thickness_) {}

Border::Border() {}
Border::~Border() {}

bool Border::operator==(const Border& other) const {
  return bottom_color_ == other.bottom_color_ &&
         left_color_ == other.left_color_ &&
         right_color_ == other.right_color_ && top_color_ == other.top_color_ &&
         thickness_ == other.thickness_;
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
    ostream << delimiter << "top=" << border.top() << ' ' << border.top_color();
    delimiter = ", ";
  }
  if (border.left() != 0.0f) {
    ostream << delimiter << "left=" << border.left() << ' '
            << border.left_color();
    delimiter = ", ";
  }
  if (border.bottom() != 0.0f) {
    ostream << delimiter << "bottom=" << border.bottom() << ' '
            << border.bottom_color();
    delimiter = ", ";
  }
  if (border.right() != 0.0f) {
    ostream << delimiter << "right=" << border.right() << ' '
            << border.right_color();
    delimiter = ", ";
  }
  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

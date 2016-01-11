// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/layout/border.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Border
//
Border::Border(const FloatColor& top_color,
               float top,
               const FloatColor& right_color,
               float right,
               const FloatColor& bottom_color,
               float bottom,
               const FloatColor& left_color,
               float left)
    : bottom_color_(bottom_color),
      left_color_(left_color),
      right_color_(right_color),
      top_color_(top_color),
      thickness_(top, left, bottom, right) {}

Border::Border(const FloatColor& color,
               float top,
               float right,
               float bottom,
               float left)
    : Border(color, top, color, right, color, bottom, color, left) {}

Border::Border(const FloatColor& color, float width)
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
  if (bottom_color_.alpha() != 0 && thickness_.bottom() != 0)
    return true;
  if (left_color_.alpha() != 0 && thickness_.left() != 0)
    return true;
  if (right_color_.alpha() != 0 && thickness_.right() != 0)
    return true;
  if (top_color_.alpha() != 0 && thickness_.top() != 0)
    return true;
  return false;
}

bool Border::IsSimple() const {
  return top() == bottom() && top() == left() && top() == right() &&
         top_color() == bottom_color() && top_color() == left_color() &&
         top_color() == right_color();
}

std::ostream& operator<<(std::ostream& ostream, const Border& border) {
  if (!border.HasValue())
    return ostream << "Border()";
  if (border.IsSimple()) {
    return ostream << "Border(width=" << border.top() << ", "
                   << border.top_color() << ')';
  }
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

}  // namespace visuals

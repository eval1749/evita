// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/point.h"

#include "common/win/size.h"

namespace common {
namespace win {

Point::Point() {
  x = y = 0;
}

Point::Point(int x, int y) {
  this->x = x;
  this->y = y;
}

Point::Point(POINTS pt) {
 x = pt.x; y = pt.y;
}

Point Point::operator+(const Size& size) const {
  return Point(x + size.width(), y + size.height());
}

Point Point::operator-(const Size& size) const {
  return Point(x - size.width(), y - size.height());
}

Size Point::operator-(const Point& other) const {
  return Size(x - other.x, y - other.y);
}

Point Point::Offset(int delta_x, int delta_y) const {
 return Point(x + delta_x, y + delta_y);
}

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point) {
  return out << "(" << point.x << "," << point.y << ")";
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point) {
  return out << *point;
}

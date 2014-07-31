// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/point.h"

#include "base/logging.h"
#include "common/win/size.h"

namespace common {
namespace win {

Point::Point(const Point& other) : data_(other.data_) {
}

Point::Point(const POINT& point) {
 data_.x = point.x;
 data_.y = point.y;
}

Point::Point(int x, int y) {
  data_.x = x;
  data_.y = y;
}

Point::Point(POINTS point) {
 data_.x = point.x;
 data_.y = point.y;
}

Point::Point() : Point(0, 0) {
}

Point& Point::operator=(const Point& other) {
  data_ = other.data_;
  return *this;
}

bool Point::operator==(const Point& other) const {
  return x() == other.x() && y() == other.y();
}

bool Point::operator!=(const Point& other) const {
  return !operator==(other);
}

Point Point::operator+(const Size& size) const {
  return Point(x() + size.width(), y() + size.height());
}

Point Point::operator-(const Size& size) const {
  return Point(x() - size.width(), y() - size.height());
}

Size Point::operator-(const Point& other) const {
  return Size(x() - other.x(), y() - other.y());
}

POINT* Point::ptr() {
  DCHECK(!x() && !y());
  return &data_;
}

Point Point::Offset(int delta_x, int delta_y) const {
 return Point(x() + delta_x, y() + delta_y);
}

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point) {
  return out << "(" << point.x() << "," << point.y() << ")";
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point) {
  return out << *point;
}

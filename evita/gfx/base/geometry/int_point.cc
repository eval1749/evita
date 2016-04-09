// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/gfx/base/geometry/int_point.h"

#include "evita/gfx/base/geometry/int_size.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// IntPoint
//
IntPoint::IntPoint(int x, int y) : x_(x), y_(y) {}
IntPoint::IntPoint(const IntPoint& other) : IntPoint(other.x_, other.y_) {}
IntPoint::IntPoint() {}
IntPoint::~IntPoint() {}

IntPoint& IntPoint::operator=(const IntPoint& other) {
  x_ = other.x_;
  y_ = other.y_;
  return *this;
}

bool IntPoint::operator==(const IntPoint& other) const {
  return x_ == other.x_ && y_ == other.y_;
}

bool IntPoint::operator!=(const IntPoint& other) const {
  return !operator==(other);
}

IntPoint IntPoint::operator+(const IntSize& size) const {
  return IntPoint(x_ + size.width(), y_ + size.height());
}

IntPoint IntPoint::operator-(const IntSize& size) const {
  return IntPoint(x_ - size.width(), y_ - size.height());
}

std::ostream& operator<<(std::ostream& ostream, const IntPoint& point) {
  return ostream << '(' << point.x() << ',' << point.y() << ')';
}

}  // namespace gfx

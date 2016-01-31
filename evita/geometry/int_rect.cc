// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/geometry/int_rect.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// IntRect
//
IntRect::IntRect(const IntPoint& origin, const IntSize& size)
    : origin_(origin), size_(size) {}
IntRect::IntRect(const IntSize& size) : IntRect(IntPoint(), size) {}
IntRect::IntRect(const IntRect& other) : IntRect(other.origin_, other.size_) {}
IntRect::IntRect() {}
IntRect::~IntRect() {}

IntRect& IntRect::operator=(const IntRect& other) {
  origin_ = other.origin_;
  size_ = other.size_;
  return *this;
}

bool IntRect::operator==(const IntRect& other) const {
  return origin_ == other.origin_ && size_ == other.size_;
}

bool IntRect::operator!=(const IntRect& other) const {
  return !operator==(other);
}

bool IntRect::Contains(int point_x, int point_y) const {
  return (point_x >= x()) && (point_x < right()) && (point_y >= y()) &&
         (point_y < bottom());
}

bool IntRect::Contains(const IntPoint& point) const {
  return Contains(point.x(), point.y());
}

bool IntRect::Contains(const IntRect& rect) const {
  return (rect.x() >= x() && rect.right() <= right() && rect.y() >= y() &&
          rect.bottom() <= bottom());
}

std::ostream& operator<<(std::ostream& ostream, const IntRect& rect) {
  return ostream << rect.origin() << '+' << rect.size();
}

}  // namespace evita

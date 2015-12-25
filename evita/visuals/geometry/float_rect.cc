// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatRect
//
FloatRect::FloatRect(const FloatPoint& origin, const FloatSize& size)
    : origin_(origin), size_(size) {}

FloatRect::FloatRect(const FloatSize& size) : size_(size) {}

FloatRect::FloatRect(const FloatRect& other)
    : FloatRect(other.origin_, other.size_) {}

FloatRect::FloatRect() {}

FloatRect::~FloatRect() {}

bool FloatRect::operator==(const FloatRect& other) const {
  return origin_ == other.origin_ && size_ == other.size_;
}

bool FloatRect::operator!=(const FloatRect& other) const {
  return !operator==(other);
}

bool FloatRect::operator<(const FloatRect& other) const {
  if (origin_ != other.origin_)
    return origin_ < other.origin_;
  if (width() == other.width())
    return height() < other.height();
  return width() < other.width();
}

bool FloatRect::Contains(float point_x, float point_y) const {
  return (point_x >= x()) && (point_x < right()) && (point_y >= y()) &&
         (point_y < bottom());
}

bool FloatRect::Contains(const FloatPoint& point) const {
  return Contains(point.x(), point.y());
}

bool FloatRect::Contains(const FloatRect& rect) const {
  return (rect.x() >= x() && rect.right() <= right() && rect.y() >= y() &&
          rect.bottom() <= bottom());
}

std::ostream& operator<<(std::ostream& ostream, const FloatRect& rect) {
  return ostream << rect.origin() << '+' << rect.size();
}

}  // namespace visuals

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GEOMETRY_INT_RECT_H_
#define EVITA_GEOMETRY_INT_RECT_H_

#include <iosfwd>

#include "evita/geometry/int_point.h"
#include "evita/geometry/int_size.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// IntRect
//
class IntRect final {
 public:
  IntRect(const IntPoint& origin, const IntSize& size);
  explicit IntRect(const IntSize& size);
  IntRect(const IntRect& other);
  IntRect();
  ~IntRect();

  IntRect& operator=(const IntRect& other);

  bool operator==(const IntRect& other) const;
  bool operator!=(const IntRect& other) const;

  const IntPoint& origin() const { return origin_; }
  const IntSize& size() const { return size_; }

  int x() const { return origin_.x(); }
  int y() const { return origin_.y(); }

  int bottom() const { return y() + height(); }
  int right() const { return x() + width(); }

  IntPoint bottom_left() const { return IntPoint(x(), bottom()); }
  IntPoint bottom_right() const { return IntPoint(right(), bottom()); }
  IntPoint top_right() const { return IntPoint(right(), y()); }

  int height() const { return size_.height(); }
  int width() const { return size_.width(); }

  // Returns true if the point identified by point_x and point_y falls
  // inside
  // this rectangle.  The point (x, y) is inside the rectangle, but the
  // point (x + width, y + height) is not.
  bool Contains(int point_x, int point_y) const;
  bool Contains(const IntPoint& point) const;
  bool Contains(const IntRect& other) const;

  // Returns true if the area of the rectangle is zero.
  bool IsEmpty() const { return size_.IsEmpty(); }

 private:
  IntPoint origin_;
  IntSize size_;
};

std::ostream& operator<<(std::ostream& ostream, const IntRect& rect);

}  // namespace evita

#endif  // EVITA_GEOMETRY_INT_RECT_H_

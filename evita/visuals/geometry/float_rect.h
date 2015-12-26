// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_RECT_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_RECT_H_

#include <iosfwd>

#include "evita/visuals/geometry/float_point.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatRect
//
class FloatRect final {
 public:
  FloatRect(const FloatPoint& origin, const FloatSize& size);
  explicit FloatRect(const FloatSize& size);
  FloatRect(const FloatRect& other);
  FloatRect();
  ~FloatRect();

  FloatRect& operator=(const FloatRect& other);

  bool operator==(const FloatRect& other) const;
  bool operator!=(const FloatRect& other) const;

  FloatRect operator+(const FloatSize& size) const;
  FloatRect operator-(const FloatSize& size) const;

  // A rect is less than another rect if its origin is less than
  // the other rect's origin. If the origins are equal, then the
  // shortest rect is less than the other. If the origin and the
  // height are equal, then the narrowest rect is less than.
  // This comparison is required to use Rects in sets, or sorted
  // vectors.
  bool operator<(const FloatRect& other) const;

  float bottom() const { return x() + width(); }
  FloatPoint bottom_left() const { return FloatPoint(x(), bottom()); }
  FloatPoint bottom_right() const { return FloatPoint(right(), bottom()); }

  float height() const { return size_.height(); }
  const FloatPoint& origin() const { return origin_; }
  float right() const { return x() + width(); }
  const FloatSize& size() const { return size_; }
  FloatPoint top_right() const { return FloatPoint(right(), y()); }
  float width() const { return size_.width(); }
  float x() const { return origin_.x(); }
  float y() const { return origin_.y(); }

  // Returns true if the point identified by point_x and point_y falls
  // inside
  // this rectangle.  The point (x, y) is inside the rectangle, but the
  // point (x + width, y + height) is not.
  bool Contains(float point_x, float point_y) const;
  bool Contains(const FloatPoint& point) const;
  bool Contains(const FloatRect& rect) const;

  // Returns true if the area of the rectangle is zero.
  bool IsEmpty() const { return size_.IsEmpty(); }

 private:
  FloatPoint origin_;
  FloatSize size_;
};

std::ostream& operator<<(std::ostream& ostream, const FloatRect& rect);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_RECT_H_

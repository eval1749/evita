// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_POINT_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_POINT_H_

#include <iosfwd>

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatPoint
//
class FloatPoint final {
 public:
  FloatPoint(float x, float y);
  FloatPoint(const FloatPoint& other);
  FloatPoint();
  ~FloatPoint();

  bool operator==(const FloatPoint& other) const;
  bool operator!=(const FloatPoint& other) const;

  // A point is less than another point if its y-value is closer
  // to the origin. If the y-values are the same, then point with
  // the x-value closer to the origin is considered less than the
  // other.
  // This comparison is required to use FloatPoint in sets, or sorted
  // vectors.
  bool operator<(const FloatPoint& other) const;

  float x() const { return x_; }
  float y() const { return y_; }

 private:
  float x_ = 0.0f;
  float y_ = 0.0f;
};

std::ostream& operator<<(std::ostream& ostream, const FloatPoint& point);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_POINT_H_

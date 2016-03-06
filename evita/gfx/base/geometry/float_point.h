// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_GEOMETRY_FLOAT_POINT_H_
#define EVITA_GFX_BASE_GEOMETRY_FLOAT_POINT_H_

#include <iosfwd>
#include <utility>

#include "evita/gfx/gfx_export.h"

namespace gfx {

class FloatSize;

//////////////////////////////////////////////////////////////////////
//
// FloatPoint
//
class GFX_EXPORT FloatPoint final {
 public:
  explicit FloatPoint(const std::pair<float, float> pair);
  FloatPoint(float x, float y);
  FloatPoint(const FloatPoint& other);
  FloatPoint();
  ~FloatPoint();

  FloatPoint& operator=(const FloatPoint& other);

  bool operator==(const FloatPoint& other) const;
  bool operator!=(const FloatPoint& other) const;

  FloatPoint operator+(const FloatSize& size) const;
  FloatPoint operator-(const FloatSize& size) const;

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

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                    const FloatPoint& point);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_GEOMETRY_FLOAT_POINT_H_

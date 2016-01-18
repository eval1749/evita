// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_

#include <array>
#include <iosfwd>

#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatQuad represents a collection of 4 points, often representing the result
// of mapping a rectangle through transforms. When initialized from a rect,
// the points are in clockwise order from top left.
//
class FloatQuad final {
 public:
  FloatQuad(const FloatPoint& point1,
            const FloatPoint& point2,
            const FloatPoint& point3,
            const FloatPoint& point4);
  explicit FloatQuad(const std::array<FloatPoint, 4>& data);
  explicit FloatQuad(const FloatRect& rect);
  FloatQuad(const FloatQuad& other);
  FloatQuad();
  ~FloatQuad();

  FloatQuad& operator=(const FloatQuad& other);

  bool operator==(const FloatQuad& other) const;
  bool operator!=(const FloatQuad& other) const;

  FloatQuad operator*(const FloatQuad& other) const;

  const std::array<FloatPoint, 4>& data() const { return data_; }
  const FloatPoint& point1() const { return data_[0]; }
  const FloatPoint& point2() const { return data_[1]; }
  const FloatPoint& point3() const { return data_[2]; }
  const FloatPoint& point4() const { return data_[3]; }

  FloatRect ComputeBoundingBox() const;

 private:
  std::array<FloatPoint, 4> data_;
};

std::ostream& operator<<(std::ostream& ostream, const FloatQuad& matrix);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_

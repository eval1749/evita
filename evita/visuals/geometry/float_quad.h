// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_

#include <array>
#include <iosfwd>

#include "evita/visuals/geometry/float_point.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatQuad
//
class FloatQuad final {
 public:
  explicit FloatQuad(const std::array<FloatPoint, 4>& data);
  FloatQuad(const FloatQuad& other);
  FloatQuad();
  ~FloatQuad();

  FloatQuad& operator=(const FloatQuad& other);

  bool operator==(const FloatQuad& other) const;
  bool operator!=(const FloatQuad& other) const;

  FloatQuad operator*(const FloatQuad& other) const;

  const std::array<FloatPoint, 4>& data() const { return data_; }

 private:
  std::array<FloatPoint, 4> data_;
};

std::ostream& operator<<(std::ostream& ostream, const FloatQuad& matrix);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_QUAD_H_

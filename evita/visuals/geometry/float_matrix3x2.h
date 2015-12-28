// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_

#include <array>
#include <iosfwd>

namespace visuals {

class FloatPoint;

//////////////////////////////////////////////////////////////////////
//
// FloatMatrix3x2 class represents a 3-by-2 matrix and provides convenience
// methods for creating matrices.
//
class FloatMatrix3x2 final {
 public:
  explicit FloatMatrix3x2(const std::array<float, 6>& data);
  FloatMatrix3x2(const FloatMatrix3x2& other);
  FloatMatrix3x2();
  ~FloatMatrix3x2();

  FloatMatrix3x2& operator=(const FloatMatrix3x2& other);

  bool operator==(const FloatMatrix3x2& other) const;
  bool operator!=(const FloatMatrix3x2& other) const;

  FloatMatrix3x2 operator*(const FloatMatrix3x2& other) const;

  const std::array<float, 6>& data() const { return data_; }

  static FloatMatrix3x2 Identity();
  static FloatMatrix3x2 Scale(float sx, float sy, const FloatPoint& center);
  static FloatMatrix3x2 Translation(float x, float y);

 private:
  std::array<float, 6> data_;
};

std::ostream& operator<<(std::ostream& ostream, const FloatMatrix3x2& matrix);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_

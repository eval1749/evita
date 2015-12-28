// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_

#include <array>
#include <iosfwd>
#include <utility>

namespace visuals {

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
  std::pair<float, float> operator*(const std::pair<float, float> pair) const;

  const std::array<float, 6>& data() const { return data_; }

  double ComputeDeterminant() const;
  FloatMatrix3x2 Inverse() const;
  bool IsIdentity() const;
  bool IsIdentityOrTranslation() const;

  static FloatMatrix3x2 Identity();

 private:
  std::array<float, 6> data_;
};

std::ostream& operator<<(std::ostream& ostream, const FloatMatrix3x2& matrix);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_MATRIX3X2_H_

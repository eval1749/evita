// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_AFFINE_TRANSFORMER_H_
#define EVITA_VISUALS_GEOMETRY_AFFINE_TRANSFORMER_H_

#include <utility>

#include "evita/visuals/geometry/float_matrix3x2.h"

namespace visuals {

class FloatPoint;
class FloatRect;
class FloatSize;

//////////////////////////////////////////////////////////////////////
//
// AffineTransformer
//
class AffineTransformer final {
 public:
  explicit AffineTransformer(const FloatMatrix3x2& matrix);
  AffineTransformer(const AffineTransformer& other);
  AffineTransformer();
  ~AffineTransformer();

  AffineTransformer& operator=(const AffineTransformer& other);

  const FloatMatrix3x2& matrix() const { return matrix_; }
  void set_matrix(const FloatMatrix3x2& matrix) { matrix_ = matrix; }

  AffineTransformer Inverse() const;
  FloatPoint MapPoint(const FloatPoint& point) const;
  FloatSize MapSize(const FloatSize& size) const;
  FloatRect MapRect(const FloatRect& rect) const;

  bool IsIdentity() const;
  bool IsIdentityOrTranslation() const;
  void Scale(float x, float y, const FloatPoint& center);
  void Translate(const FloatPoint& point);
  void Translate(float x, float y);

 private:
  FloatMatrix3x2 matrix_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_AFFINE_TRANSFORMER_H_

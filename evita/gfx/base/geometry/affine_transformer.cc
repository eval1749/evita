// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/base/geometry/affine_transformer.h"

#include "evita/gfx/base/geometry/float_quad.h"
#include "evita/gfx/base/geometry/float_rect.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// AffineTransformer
//
AffineTransformer::AffineTransformer(const FloatMatrix3x2& matrix)
    : matrix_(matrix) {}

AffineTransformer::AffineTransformer(const AffineTransformer& other)
    : AffineTransformer(other.matrix_) {}

AffineTransformer::AffineTransformer()
    : AffineTransformer(FloatMatrix3x2::Identity()) {}

AffineTransformer::~AffineTransformer() {}

AffineTransformer& AffineTransformer::operator=(
    const AffineTransformer& other) {
  matrix_ = other.matrix_;
  return *this;
}

AffineTransformer AffineTransformer::Inverse() const {
  return AffineTransformer(matrix_.Inverse());
}

FloatPoint AffineTransformer::MapPoint(const FloatPoint& point) const {
  return FloatPoint(matrix_ * std::make_pair(point.x(), point.y()));
}

FloatSize AffineTransformer::MapSize(const FloatSize& size) const {
  return FloatSize(matrix_ * std::make_pair(size.width(), size.height()));
}

FloatRect AffineTransformer::MapRect(const FloatRect& rect) const {
  if (IsIdentityOrTranslation()) {
    const auto offset_x = matrix_.data()[4];
    const auto offset_y = matrix_.data()[5];
    if (offset_x == 0 && offset_y == 0)
      return rect;
    return FloatRect(FloatPoint(rect.x() + offset_x, rect.y() + offset_y),
                     rect.size());
  }
  return FloatQuad(MapPoint(rect.origin()), MapPoint(rect.top_right()),
                   MapPoint(rect.bottom_left()), MapPoint(rect.bottom_right()))
      .ComputeBoundingBox();
}

bool AffineTransformer::IsIdentity() const {
  return matrix_.IsIdentity();
}

bool AffineTransformer::IsIdentityOrTranslation() const {
  return matrix_.IsIdentityOrTranslation();
}

void AffineTransformer::Scale(float sx, float sy, const FloatPoint& center) {
  matrix_ =
      matrix_ * FloatMatrix3x2({sx, 0, 0, sy, center.x() - sx * center.x(),
                                center.y() - sy * center.y()});
}

void AffineTransformer::Translate(const FloatPoint& point) {
  Translate(point.x(), point.y());
}

void AffineTransformer::Translate(float x, float y) {
  matrix_ = matrix_ * FloatMatrix3x2({1, 0, 0, 1, x, y});
}

}  // namespace gfx

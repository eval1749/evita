// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_GEOMETRY_FLOAT_SIZE_H_
#define EVITA_GFX_BASE_GEOMETRY_FLOAT_SIZE_H_

#include <iosfwd>
#include <utility>

#include "evita/gfx/gfx_export.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FloatSize
//
class GFX_EXPORT FloatSize final {
 public:
  explicit FloatSize(const std::pair<float, float> pair);
  FloatSize(float width, float height);
  FloatSize(const FloatSize& other);
  FloatSize();
  ~FloatSize();

  FloatSize& operator=(const FloatSize& other);

  bool operator==(const FloatSize& other) const;
  bool operator!=(const FloatSize& other) const;

  FloatSize operator+(const FloatSize& other) const;
  FloatSize operator-(const FloatSize& other) const;

  float height() const { return height_; }
  float width() const { return width_; }

  bool IsEmpty() const { return width_ == 0 || height_ == 0; }

 private:
  float height_ = 0.0f;
  float width_ = 0.0f;
};

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                    const FloatSize& size);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_GEOMETRY_FLOAT_SIZE_H_

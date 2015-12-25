// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_GEOMETRY_FLOAT_SIZE_H_
#define EVITA_VISUALS_GEOMETRY_FLOAT_SIZE_H_

#include <iosfwd>

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatSize
//
class FloatSize final {
 public:
  FloatSize(float width, float height);
  FloatSize(const FloatSize& other);
  FloatSize();
  ~FloatSize();

  bool operator==(const FloatSize& other) const;
  bool operator!=(const FloatSize& other) const;

  float height() const { return height_; }
  float width() const { return width_; }

  bool IsEmpty() const { return width_ == 0 && height_ == 0; }

 private:
  const float height_ = 0.0f;
  const float width_ = 0.0f;
};

std::ostream& operator<<(std::ostream& ostream, const FloatSize& size);

}  // namespace visuals

#endif  // EVITA_VISUALS_GEOMETRY_FLOAT_SIZE_H_

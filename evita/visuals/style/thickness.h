// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_THICKNESS_H_
#define EVITA_VISUALS_STYLE_THICKNESS_H_

#include <iosfwd>

#include "evita/visuals/geometry/float_size.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Thickness
//
class Thickness {
 public:
  Thickness(float top, float left, float bottom, float right);
  Thickness(const Thickness& other);
  Thickness();
  ~Thickness();

  bool operator==(const Thickness& other) const;
  bool operator!=(const Thickness& other) const;

  float bottom() const { return bottom_; }
  FloatSize bottom_right() const;
  float left() const { return left_; }
  float right() const { return right_; }
  FloatSize size() const { return FloatSize(left_ + right_, top_ + bottom_); }
  float top() const { return top_; }
  FloatSize top_left() const;

  bool HasValue() const;

 private:
  float bottom_ = 0.0f;
  float left_ = 0.0f;
  float right_ = 0.0f;
  float top_ = 0.0f;
};

std::ostream& operator<<(std::ostream& ostream, const Thickness& thickness);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_THICKNESS_H_

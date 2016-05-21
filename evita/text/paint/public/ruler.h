// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_PAINT_PUBLIC_RULER_H_
#define EVITA_TEXT_PAINT_PUBLIC_RULER_H_

#include "evita/gfx/color_f.h"

namespace paint {

class Ruler final {
 public:
  Ruler(float x_point, float size, const gfx::ColorF& color);
  Ruler(const Ruler&);
  ~Ruler();

  Ruler& operator=(const Ruler&) = delete;

  const gfx::ColorF color() const { return color_; }
  float size() const { return size_; }
  float x_point() const { return x_point_; }

 private:
  const gfx::ColorF color_;
  const float size_;
  const float x_point_;
};

}  // namespace paint

#endif  // EVITA_TEXT_PAINT_PUBLIC_RULER_H_

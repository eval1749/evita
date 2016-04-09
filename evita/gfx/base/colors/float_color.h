// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_COLORS_FLOAT_COLOR_H_
#define EVITA_GFX_BASE_COLORS_FLOAT_COLOR_H_

#include <stdint.h>

#include <iosfwd>

#include "evita/gfx/gfx_export.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FloatColor
//
class GFX_EXPORT FloatColor final {
 public:
  FloatColor(float red, float green, float blue, float alpha = 1);
  FloatColor(const FloatColor& other);
  FloatColor();
  ~FloatColor();

  bool operator==(const FloatColor& other) const;
  bool operator!=(const FloatColor& other) const;

  float alpha() const { return alpha_; }
  float blue() const { return blue_; }
  float green() const { return green_; }
  float red() const { return red_; }

  uint32_t ToRgba() const;

 private:
  float alpha_ = 0;
  float blue_ = 0;
  float green_ = 0;
  float red_ = 0;
};

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                    const FloatColor& color);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_COLORS_FLOAT_COLOR_H_

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_COLORS_INT8_COLOR_H_
#define EVITA_GFX_BASE_COLORS_INT8_COLOR_H_

#include <iosfwd>

#include "evita/gfx/gfx_export.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// Int8Color
//
class GFX_EXPORT Int8Color final {
 public:
  Int8Color(int red, int green, int blue, int alpha = 255);
  Int8Color(const Int8Color& other);
  Int8Color();
  ~Int8Color();

  bool operator==(const Int8Color& other) const;
  bool operator!=(const Int8Color& other) const;

  int alpha() const { return alpha_; }
  int blue() const { return blue_; }
  int green() const { return green_; }
  int red() const { return red_; }

 private:
  int alpha_ = 0;
  int blue_ = 0;
  int green_ = 0;
  int red_ = 0;
};

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                    const Int8Color& color);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_COLORS_INT8_COLOR_H_

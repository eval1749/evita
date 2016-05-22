// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_VALUES_COLOR_VALUE_H_
#define EVITA_CSS_VALUES_COLOR_VALUE_H_

#include <iosfwd>

#include "evita/gfx/base/colors/float_color.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// ColorValue
//
class ColorValue final {
 public:
  ColorValue(float red, float green, float blue, float alpha = 1);
  explicit ColorValue(const gfx::FloatColor& value);
  ColorValue(const ColorValue& other);
  ColorValue();
  ~ColorValue();

  bool operator==(const ColorValue& other) const;
  bool operator!=(const ColorValue& other) const;

  const gfx::FloatColor& value() const { return value_; }

  // Returns |ColorValue| object with RGBA value from 0 to 255 RGB values.
  static ColorValue Rgba(int red, int green, int blue, float alpha = 1);

 private:
  gfx::FloatColor value_;
};

std::ostream& operator<<(std::ostream& ostream, const ColorValue& color);

}  // namespace css

#endif  // EVITA_CSS_VALUES_COLOR_VALUE_H_

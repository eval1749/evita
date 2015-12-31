// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_COLOR_H_
#define EVITA_VISUALS_CSS_COLOR_H_

#include <iosfwd>

#include "evita/visuals/css/float_color.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Color
//
class Color final {
 public:
  Color(float red, float green, float blue, float alpha = 1);
  explicit Color(const FloatColor& value);
  Color(const Color& other);
  Color();
  ~Color();

  bool operator==(const Color& other) const;
  bool operator!=(const Color& other) const;

  const FloatColor& value() const { return value_; }

  // Returns |Color| object with RGBA value from 0 to 255 RGB values.
  static Color Rgba(int red, int green, int blue, float alpha = 1);

 private:
  FloatColor value_;
};

std::ostream& operator<<(std::ostream& ostream, const Color& color);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_COLOR_H_

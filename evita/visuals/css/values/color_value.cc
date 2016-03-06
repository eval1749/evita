// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/values/color_value.h"

namespace visuals {
namespace css {

ColorValue::ColorValue(float red, float green, float blue, float alpha)
    : value_(gfx::FloatColor(red, green, blue, alpha)) {}

ColorValue::ColorValue(const gfx::FloatColor& value) : value_(value) {}
ColorValue::ColorValue(const ColorValue& other) : value_(other.value_) {}
ColorValue::ColorValue() {}
ColorValue::~ColorValue() {}

bool ColorValue::operator==(const ColorValue& other) const {
  return value_ == other.value_;
}

bool ColorValue::operator!=(const ColorValue& other) const {
  return !operator==(other);
}

// static
ColorValue ColorValue::Rgba(int red, int green, int blue, float alpha) {
  return ColorValue(static_cast<float>(red) / 255,
                    static_cast<float>(green) / 255,
                    static_cast<float>(blue) / 255, alpha);
}

std::ostream& operator<<(std::ostream& ostream, const ColorValue& color) {
  const auto& value = color.value();
  ostream << "ColorValue(" << value.red() << ", " << value.green() << ", "
          << value.blue();
  if (value.alpha() != 1)
    ostream << ", " << value.alpha();
  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

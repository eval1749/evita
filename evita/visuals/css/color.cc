// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/color.h"

namespace visuals {
namespace css {

Color::Color(float red, float green, float blue, float alpha)
    : value_(FloatColor(red, green, blue, alpha)) {}

Color::Color(const FloatColor& value) : value_(value) {}
Color::Color(const Color& other) : value_(other.value_) {}
Color::Color() {}
Color::~Color() {}

bool Color::operator==(const Color& other) const {
  return value_ == other.value_;
}

bool Color::operator!=(const Color& other) const {
  return !operator==(other);
}

// static
Color Color::Rgba(int red, int green, int blue, float alpha) {
  return Color(red / 255, green / 255, blue / 255, alpha);
}

std::ostream& operator<<(std::ostream& ostream, const Color& color) {
  const auto& value = color.value();
  ostream << "Color(" << value.red() << ", " << value.green() << ", "
          << value.blue();
  if (value.alpha() != 1)
    ostream << ", " << value.alpha();
  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/base/colors/float_color.h"

#include "base/logging.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FloatColor
//
FloatColor::FloatColor(float red, float green, float blue, float alpha)
    : alpha_(alpha), blue_(blue), green_(green), red_(red) {
  DCHECK_GE(alpha_, 0.0f);
  DCHECK_LE(alpha_, 1.0f);
  DCHECK_GE(blue_, 0.0f);
  DCHECK_LE(blue_, 1.0f);
  DCHECK_GE(green_, 0.0f);
  DCHECK_LE(green_, 1.0f);
  DCHECK_GE(red_, 0.0f);
  DCHECK_LE(red_, 1.0f);
}

FloatColor::FloatColor(const FloatColor& other)
    : FloatColor(other.red_, other.green_, other.blue_, other.alpha_) {}

FloatColor::FloatColor() {}
FloatColor::~FloatColor() {}

bool FloatColor::operator==(const FloatColor& other) const {
  if (this == &other)
    return true;
  return alpha_ == other.alpha_ && blue_ == other.blue_ &&
         green_ == other.green_ && red_ == other.red_;
}

bool FloatColor::operator!=(const FloatColor& other) const {
  return !operator==(other);
}

uint32_t FloatColor::ToRgba() const {
  const auto red = static_cast<uint32_t>(red_ * 255);
  const auto green = static_cast<uint32_t>(green_ * 255);
  const auto blue = static_cast<uint32_t>(blue_ * 255);
  const auto alpha = static_cast<uint32_t>(alpha_ * 255);
  return (red << 24) | (green << 16) | (blue << 8) | alpha;
}

std::ostream& operator<<(std::ostream& ostream, const FloatColor& color) {
  ostream << "FloatColor(" << color.red() << ", " << color.green() << ", "
          << color.blue();
  if (color.alpha() != 1)
    ostream << ", " << color.alpha();
  return ostream << ')';
}

}  // namespace gfx

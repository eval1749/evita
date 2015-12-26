// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/float_color.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatColor
//
FloatColor::FloatColor(float red, float green, float blue, float alpha)
    : alpha_(alpha), blue_(blue), green_(green), red_(red) {}

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

}  // namespace visuals

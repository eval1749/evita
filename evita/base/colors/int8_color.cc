// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/colors/int8_color.h"

#include "base/logging.h"

namespace base {

//////////////////////////////////////////////////////////////////////
//
// Int8Color
//
Int8Color::Int8Color(int red, int green, int blue, int alpha)
    : alpha_(alpha), blue_(blue), green_(green), red_(red) {
  DCHECK_GE(alpha_, 0);
  DCHECK_LE(alpha_, 255);
  DCHECK_GE(blue_, 0);
  DCHECK_LE(blue_, 255);
  DCHECK_GE(green_, 0);
  DCHECK_LE(green_, 255);
  DCHECK_GE(red_, 0);
  DCHECK_LE(red_, 255);
}

Int8Color::Int8Color(const Int8Color& other)
    : Int8Color(other.red_, other.green_, other.blue_, other.alpha_) {}

Int8Color::Int8Color() {}
Int8Color::~Int8Color() {}

bool Int8Color::operator==(const Int8Color& other) const {
  if (this == &other)
    return true;
  return alpha_ == other.alpha_ && blue_ == other.blue_ &&
         green_ == other.green_ && red_ == other.red_;
}

bool Int8Color::operator!=(const Int8Color& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Int8Color& color) {
  ostream << "Int8Color(" << color.red() << ", " << color.green() << ", "
          << color.blue();
  if (color.alpha() != 1)
    ostream << ", " << color.alpha();
  return ostream << ')';
}

}  // namespace base

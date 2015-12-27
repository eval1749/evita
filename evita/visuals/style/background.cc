// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/style/background.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Background
//
Background::Background(const FloatColor& color) : color_(color) {}

Background::Background(const Background& other) : Background(other.color_) {}

Background::Background() {}
Background::~Background() {}

bool Background::operator==(const Background& other) const {
  if (this == &other)
    return true;
  return color_ == other.color_;
}

bool Background::operator!=(const Background& other) const {
  return !operator==(other);
}

bool Background::HasValue() const {
  return color_ != FloatColor();
}

std::ostream& operator<<(std::ostream& ostream, const Background& background) {
  ostream << "Background(";
  if (background.HasValue())
    ostream << background.color();
  return ostream << ')';
}

}  // namespace visuals

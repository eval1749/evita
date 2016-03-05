// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/values/dimension.h"

#include "evita/visuals/css/values/unit.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Dimension
//
Dimension::Dimension(float number, Unit unit) : number_(number), unit_(unit) {}

Dimension::Dimension(const Dimension& other)
    : Dimension(other.number_, other.unit_) {}

Dimension::~Dimension() {}

bool Dimension::is_absolute_length() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::cm) &&
         value <= static_cast<int>(Unit::q);
}

bool Dimension::is_angle() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::deg) &&
         value <= static_cast<int>(Unit::turn);
}

bool Dimension::is_duration() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::ms) &&
         value <= static_cast<int>(Unit::s);
}

bool Dimension::is_font_relative_length() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::ch) &&
         value <= static_cast<int>(Unit::rem);
}

bool Dimension::is_frequency() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::Hz) &&
         value <= static_cast<int>(Unit::kHz);
}

bool Dimension::is_length() const {
  return is_absolute_length() || is_font_relative_length() ||
         is_viewport_percentage_length();
}

bool Dimension::is_resolution() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::dpi) &&
         value <= static_cast<int>(Unit::dppx);
}

bool Dimension::is_viewport_percentage_length() const {
  const auto value = static_cast<int>(unit_);
  return value >= static_cast<int>(Unit::vh) &&
         value <= static_cast<int>(Unit::vw);
}

Dimension& Dimension::operator=(const Dimension& other) {
  number_ = other.number_;
  unit_ = other.unit_;
  return *this;
}

bool Dimension::operator==(const Dimension& other) const {
  return number_ == other.number_ && unit_ == other.unit_;
}

bool Dimension::operator!=(const Dimension& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Dimension& dimension) {
  return ostream << dimension.number() << dimension.unit();
}

}  // namespace css
}  // namespace visuals

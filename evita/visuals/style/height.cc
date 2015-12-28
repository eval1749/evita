// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/height.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Height::Kind
//
enum class Height::Kind {
  Auto,
  Length,
  Percentage,
};

//////////////////////////////////////////////////////////////////////
//
// Height
//
Height::Height(Kind kind, float value) : kind_(kind), value_(value) {
  if (IsLength() || IsPercentage())
    return;
  DCHECK_EQ(0.0f, value_);
}

Height::Height(const Height& other) : Height(other.kind_, other.value_) {}
Height::Height() : Height(Kind::Auto, 0) {}
Height::~Height() {}

Height& Height::operator=(const Height& other) {
  kind_ = other.kind_;
  value_ = other.value_;
  return *this;
}

bool Height::operator==(const Height& other) const {
  return kind_ == other.kind_ && value_ == other.value_;
}

bool Height::operator!=(const Height& other) const {
  return !operator==(other);
}

float Height::value() const {
  DCHECK(IsLength() || IsPercentage());
  return value_;
}

bool Height::IsAuto() const {
  return kind_ == Kind::Auto;
}
bool Height::IsLength() const {
  return kind_ == Kind::Length;
}
bool Height::IsPercentage() const {
  return kind_ == Kind::Percentage;
}

// static
Height Height::Auto() {
  return Height();
}

// static
Height Height::Length(float value) {
  return Height(Kind::Length, value);
}

// static
Height Height::Percentage(float value) {
  return Height(Kind::Percentage, value);
}

std::ostream& operator<<(std::ostream& ostream, const Height& height) {
  ostream << "Height(";
  if (height.IsAuto())
    return ostream << "auto\u0029";
  if (height.IsLength())
    return ostream << height.value() << '\u0029';
  if (height.IsPercentage())
    return ostream << height.value() << "%\u0029";
  return ostream << "Invalid)";
}

}  // namespace visuals

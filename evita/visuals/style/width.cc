// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/width.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Width::Kind
//
enum class Width::Kind {
  Auto,
  Length,
  Percentage,
};

//////////////////////////////////////////////////////////////////////
//
// Width
//
Width::Width(Kind kind, float value) : kind_(kind), value_(value) {
  if (IsLength() || IsPercentage())
    return;
  DCHECK_EQ(0.0f, value_);
}

Width::Width(const Width& other) : Width(other.kind_, other.value_) {}
Width::Width() : Width(Kind::Auto, 0) {}
Width::~Width() {}

Width& Width::operator=(const Width& other) {
  kind_ = other.kind_;
  value_ = other.value_;
  return *this;
}

bool Width::operator==(const Width& other) const {
  return kind_ == other.kind_ && value_ == other.value_;
}

bool Width::operator!=(const Width& other) const {
  return !operator==(other);
}

float Width::value() const {
  DCHECK(IsLength() || IsPercentage());
  return value_;
}

bool Width::IsAuto() const {
  return kind_ == Kind::Auto;
}
bool Width::IsLength() const {
  return kind_ == Kind::Length;
}
bool Width::IsPercentage() const {
  return kind_ == Kind::Percentage;
}

// static
Width Width::Auto() {
  return Width();
}

// static
Width Width::Length(float value) {
  return Width(Kind::Length, value);
}

// static
Width Width::Percentage(float value) {
  return Width(Kind::Percentage, value);
}

std::ostream& operator<<(std::ostream& ostream, const Width& width) {
  ostream << "Width(";
  if (width.IsAuto())
    return ostream << "auto\u0029";
  if (width.IsLength())
    return ostream << width.value() << '\u0029';
  if (width.IsPercentage())
    return ostream << width.value() << "%\u0029";
  return ostream << "Invalid)";
}

}  // namespace visuals

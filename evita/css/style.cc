// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/css/style.h"

#include "base/logging.h"
#include "evita/css/properties.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// Style
//
Style::Style(const Style& other) {
  *this = other;
}

Style::Style(Style&& other) {
  *this = std::move(other);
}

Style::Style() {}
Style::~Style() {}

Style& Style::operator=(const Style& other) {
  properties_ = other.properties_;
  return *this;
}

Style& Style::operator=(Style&& other) {
  properties_ = std::move(other.properties_);
  return *this;
}

bool Style::operator==(const Style& other) const {
  if (this == &other)
    return true;
  return properties_ == other.properties_;
}

bool Style::operator!=(const Style& other) const {
  return !operator==(other);
}

#define V(Name, name, type, text)                       \
  type Style::name() const {                            \
    DCHECK(has_##name());                               \
    return type(properties_.ValueOf(PropertyId::Name)); \
  }                                                     \
                                                        \
  bool Style::has_##name() const {                      \
    return properties_.Contains(PropertyId::Name);      \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

std::ostream& operator<<(std::ostream& ostream, const Style& style) {
  return ostream << "Style(" << style.properties() << ')';
}

}  // namespace css

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/style.h"

#include "base/logging.h"
#include "evita/visuals/css/properties.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Style
//
#define V(Name, name, type, text) name##_(other.name##_),
Style::Style(const Style& other) : FOR_EACH_VISUAL_CSS_PROPERTY(V) dummy_(0) {}
#undef V

Style::Style() {}
Style::~Style() {}

bool Style::operator==(const Style& other) const {
  if (this == &other)
    return true;
#define V(Name, name, type, text)             \
  if (has_##name() != other.has_##name())     \
    return false;                             \
  if (has_##name() && name() != other.name()) \
    return false;

  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  return true;
}

bool Style::operator!=(const Style& other) const {
  return !operator==(other);
}

#define V(Name, name, type, text)                  \
  const type& Style::name() const {                \
    DCHECK(has_##name());                          \
    return name##_;                                \
  }                                                \
                                                   \
  bool Style::has_##name() const {                 \
    return contains_.count(PropertyId::Name) == 1; \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

std::ostream& operator<<(std::ostream& ostream, const Style& style) {
  ostream << "Style(";
  auto delimiter = "";
#define V(Name, name, type, text)         \
  if (style.has_##name()) {               \
    ostream << delimiter << style.name(); \
    delimiter = ", ";                     \
  }
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

  return ostream << ')';
}

}  // namespace css
}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style.h"

#include "base/logging.h"
#include "evita/visuals/style/style_properties.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Style
//
#define V(Name, name, type, text) name##_(other.name##_),
Style::Style(const Style& other)
    : FOR_EACH_VISUAL_STYLE_PROPERTY(V) dummy_(0) {}
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

  FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V
  return true;
}

bool Style::operator!=(const Style& other) const {
  return !operator==(other);
}

#define V(Name, name, type, text)                       \
  type Style::name() const {                            \
    DCHECK(has_##name());                               \
    return name##_;                                     \
  }                                                     \
                                                        \
  bool Style::has_##name() const {                      \
    return contains_.count(StylePropertyId::Name) == 1; \
  }

FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V

}  // namespace visuals

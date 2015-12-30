// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>

#include "evita/visuals/css/property.h"

#include "evita/visuals/css/properties_forward.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Property
//
Property::Property() {}
Property::~Property() {}

const char* Property::name() const {
  static const char* const names[] = {
#define V(Name, name, type, text) text,
      FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  };
  const auto& it = std::begin(names) + static_cast<size_t>(id());
  return it < std::end(names) ? *it : "???";
}

}  // namespace css
}  // namespace visuals

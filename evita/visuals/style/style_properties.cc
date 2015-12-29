// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "evita/visuals/style/style_properties.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, StylePropertyId property_id) {
  static const char* texts[] = {
#define V(Name, name, type, text) #Name,
      FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V
  };

  ostream << "StylePropertyId::";
  const auto& it = std::begin(texts) + static_cast<size_t>(property_id);
  if (it >= std::end(texts))
    return ostream << "Invalid";
  return ostream << *it;
}

}  // namespace css
}  // namespace visuals

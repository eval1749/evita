// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <ostream>

#include "evita/visuals/css/properties.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, PropertyId property_id) {
  const char* const texts[] = {
{% for property in properties %}
  "{{property.name}}",
{% endfor %}
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(property_id);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace css
}  // namespace visuals

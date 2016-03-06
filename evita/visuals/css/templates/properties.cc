// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, PropertyId property_id) {
  return ostream << Property::AsciiNameOf(property_id);
}

}  // namespace css
}  // namespace visuals

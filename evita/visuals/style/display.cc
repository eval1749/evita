// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <ostream>

#include "evita/visuals/style/display.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, Display display) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_VISUALS_ENUM_DISPLAY(V)
#undef V
  };
  const auto it = std::begin(texts) + static_cast<size_t>(display);
  if (it >= std::end(texts))
    return ostream << "visuals::Display::Invalid";
  return ostream << "visuals::Display::" << *it;
}

}  // namespace css
}  // namespace visuals

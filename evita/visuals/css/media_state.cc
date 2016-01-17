// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <iterator>
#include <ostream>

#include "evita/visuals/css/media_state.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, MediaState media_state) {
  static const char* const texts[] = {
#define V(name) #name,
      FOR_EACH_CSS_MEDIA_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(media_state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

}  // namespace css
}  // namespace visuals

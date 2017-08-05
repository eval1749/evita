// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <iterator>
#include <ostream>

#include "evita/css/media_type.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// MediaType
//
std::ostream& operator<<(std::ostream& ostream, MediaType media_type) {
  const char* texts[] = {
#define V(name) #name,
      FOR_EACH_VISUALS_CSS_MEDIA_TYPE(V)
#undef V
  };
  auto* const* it = std::begin(texts) + static_cast<size_t>(media_type);
  return ostream << (it < std::end(texts) ? *it : "???");
}

}  // namespace css

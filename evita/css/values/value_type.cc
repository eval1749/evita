// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/css/values/value_type.h"

namespace css {

std::ostream& operator<<(std::ostream& ostream, ValueType type) {
  static const char* const texts[] = {
#define V(name) #name,
      FOR_EACH_CSS_VALUE_TYPE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(type);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace css

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/dom/public/caret_shape.h"

namespace domapi {

std::ostream& operator<<(std::ostream& ostream, CaretShape caret_shape) {
  const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_DOMAPI_CARET_SHAPE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(caret_shape);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace domapi

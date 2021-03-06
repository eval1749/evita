// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "evita/dom/public/scroll_bar_part.h"

namespace domapi {

std::ostream& operator<<(std::ostream& ostream, ScrollBarPart part) {
  static const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_SCROLL_BAR_PART(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(part);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "ScrollBarPart::???";
  return ostream << "ScrollBarPart::" << *it;
}

}  // namespace domapi

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/dom/public/cursor.h"

namespace domapi {

std::ostream& operator<<(std::ostream& ostream, CursorId cursor_id) {
  static const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_DOMAPI_CURSOR(V)
#undef V
  };
  auto* const* const it = std::begin(texts) + static_cast<size_t>(cursor_id);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "CursorId::???";
  return ostream << "CursorId::" << *it;
}

}  // namespace domapi

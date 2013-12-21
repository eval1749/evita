// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/win/rect.h"

// TODO: Move |#undef max| to another place.
#undef max
// TODO: Move |#undef min| to another place.
#undef min
#include <algorithm>

namespace base {
namespace win {

Rect Rect::Intersect(const Rect& other) const {
  return Rect(std::max(left, other.left), std::max(top, other.top),
              std::min(right, other.right), std::min(bottom, other.bottom));
}

} // namespace win
} // namespace base

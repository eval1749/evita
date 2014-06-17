// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/rect.h"

#include <algorithm>

namespace common {
namespace win {

bool Rect::Contains(const Rect& other) const {
  return left <= other.left && right >= other.right &&
         top <= other.top && bottom >= other.bottom;
}

Rect Rect::Intersect(const Rect& other) const {
  return Rect(std::max(left, other.left), std::max(top, other.top),
              std::min(right, other.right), std::min(bottom, other.bottom));
}

void Rect::Unite(const Rect& other) {
  if (other.empty())
    return;
  if (empty()) {
    *this = other;
    return;
  }
  left = std::min(left, other.left);
  top = std::min(top, other.top);
  right = std::max(right, other.right);
  bottom = std::max(bottom, other.bottom);
}

} // namespace win
} // namespace common

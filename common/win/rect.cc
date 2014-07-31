// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/rect.h"

#include <algorithm>

namespace common {
namespace win {

Rect::Rect(const Point& origin, const Point& bottom_right) {
  left = origin.x;
  top = origin.y;
  right = bottom_right.x;
  bottom = bottom_right.y;
}

Rect::Rect(const Point& origin, const Size& size) {
  left = origin.x;
  top = origin.y;
  right = left + size.cx;
  bottom = top + size.cy;
}

Rect::Rect(const Size& size) {
  left = top = 0;
  right = size.cx;
  bottom = size.cy;
}

Rect::Rect(int l, int t, int r, int b) {
  left = l; right= r; top= t; bottom= b;
}

Rect::Rect(const RECT& other) {
  left = other.left;
  right = other.right;
  top = other.top;
  bottom = other.bottom;
}

Rect::Rect() {
  left = right = top = bottom =0;
}


bool Rect::Contains(const Point& pt) const {
  return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
}

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

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect& rect) {
  return out << rect.origin() << "+" << rect.size();
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect* rect) {
  return out << *rect;
}

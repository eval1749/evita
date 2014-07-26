// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_rect_h)
#define INCLUDE_common_win_rect_h

#include "common/common_export.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace win {

struct COMMON_EXPORT Rect : RECT {
  Rect() {
    left = right = top = bottom =0;
  }

  Rect(int l, int t, int r, int b) {
    left = l; right= r; top= t; bottom= b;
  }

  Rect(const Point& origin, const Size& size) {
    left = origin.x;
    top = origin.y;
    right = left + size.cx;
    bottom = top + size.cy;
  }

  Rect(const Point& origin, const Point& bottom_right) {
    left = origin.x;
    top = origin.y;
    right = bottom_right.x;
    bottom = bottom_right.y;
  }

  Rect(RECT rc) {
    left = rc.left; right = rc.right; top = rc.top; bottom = rc.bottom;
  }

  bool operator==(const Rect& other) const;
  bool operator!=(const Rect& other) const;
  bool operator<(const Rect& other) const;
  bool operator<=(const Rect& other) const;
  bool operator>(const Rect& other) const;
  bool operator>=(const Rect& other) const;

  explicit operator bool() const { return !empty(); }
  int area() const { return width() * height(); }
  Point bottom_right() const { return Point(right, bottom); }
  bool empty() const { return width() <= 0 || height() <= 0; }
  int height() const { return bottom - top; }
  Point origin() const { return Point(left, top); }
  void set_origin(const Point& new_origin);
  int width() const { return right - left; }
  Size size() const { return Size(width(), height()); }

  bool Contains(const Point& pt) const {
    return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
  }

  bool Contains(const Rect& other) const;
  Rect Intersect(const Rect&) const;
  void Unite(const Rect& other);
};

// Rect inline functions

inline bool Rect::operator==(const Rect& other) const {
  return left == other.left && top == other.top &&
         right == other.right && bottom == other.bottom;
}

inline bool Rect::operator!=(const Rect& other) const {
  return !operator==(other);
}

inline bool Rect::operator<(const Rect& other) const {
  return area() < other.area();
}

inline bool Rect::operator<=(const Rect& other) const {
  return area() <= other.area();
}

inline bool Rect::operator>(const Rect& other) const {
  return area() > other.area();
}

inline bool Rect::operator>=(const Rect& other) const {
  return area() >= other.area();
}

inline void Rect::set_origin(const Point& new_origin) {
  auto const width = this->width();
  auto const height = this->height();
  left = new_origin.x;
  top = new_origin.y;
  right = left + width;
  bottom = top + height;
}

#define DEBUG_RECT_FORMAT "(%d,%d)+%dx%d"
#define DEBUG_RECT_ARG(mp_rect) \
 (mp_rect).left, (mp_rect).top, (mp_rect).width(), (mp_rect).height()

} // namespace win
} // namespace common

#endif //!defined(INCLUDE_common_win_rect_h)

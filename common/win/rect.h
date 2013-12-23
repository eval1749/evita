// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_win_rect_h)
#define INCLUDE_base_win_rect_h

#include "base/base_export.h"
#include "base/win/point.h"
#include "base/win/size.h"

namespace base {
namespace win {

struct BASE_EXPORT Rect : RECT {
  Rect() {
    left = right = top = bottom =0;
  }

  Rect(int l, int t, int r, int b) {
    left = l; right= r; top= t; bottom= b;
  }

  Rect(const Point& left_top, const Size& size) {
    left = left_top.x;
    top = left_top.y;
    right = left + size.cx;
    bottom = top + size.cy;
  }

  Rect(RECT rc) {
    left = rc.left; right = rc.right; top = rc.top; bottom = rc.bottom;
  }

  operator bool() const { return !is_empty(); }
  bool is_empty() const { return width() <= 0 || height() <= 0; }
  int height() const { return bottom - top; }
  Point left_top() const { return Point(left, top); }
  int width() const { return right - left; }
  Size size() const { return Size(width(), height()); }

  bool Contains(const Point& pt) const {
    return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
  }

  Rect Intersect(const Rect&) const;
};

#define DEBUG_RECT_FORMAT "(%d,%d)+%dx%d"
#define DEBUG_RECT_ARG(mp_rect) \
 (mp_rect).left, (mp_rect).top, (mp_rect).width(), (mp_rect).height()

} // namespace win
} // namespace base

#endif //!defined(INCLUDE_base_win_rect_h)

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_RECT_H_
#define COMMON_WIN_RECT_H_

#include <ostream>

#include "common/common_export.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace win {

class COMMON_EXPORT Rect final {
 public:
  Rect(const Point& origin, const Point& bottom_right);
  Rect(const Point& origin, const Size& size);
  explicit Rect(const Size& size);
  explicit Rect(const RECT& other);
  Rect(const Rect& other);
  Rect(int l, int t, int r, int b);
  Rect();
  ~Rect();

  explicit operator RECT() const;
  Rect& operator=(const Rect& other);
  Rect& operator+=(const Rect& other);
  bool operator==(const Rect& other) const;
  bool operator!=(const Rect& other) const;
  bool operator<(const Rect& other) const;
  bool operator<=(const Rect& other) const;
  bool operator>(const Rect& other) const;
  bool operator>=(const Rect& other) const;
  Rect operator+(const Rect& other) const { return Union(other); }
  Rect operator-(const Size& size) const;

  int area() const { return width() * height(); }
  int bottom() const { return data_.bottom; }
  Point bottom_left() const { return Point(left(), bottom()); }
  Point bottom_right() const { return Point(right(), bottom()); }
  bool empty() const { return width() <= 0 || height() <= 0; }
  int height() const { return bottom() - top(); }
  int left() const { return data_.left; }
  Point origin() const { return Point(left(), top()); }
  void set_origin(const Point& new_origin);
  int right() const { return data_.right; }
  Size size() const { return Size(width(), height()); }
  int top() const { return data_.top; }
  Point top_right() const { return Point(right(), top()); }
  int width() const { return right() - left(); }

  bool Contains(const Point& pt) const;
  bool Contains(const Rect& other) const;
  Rect Intersect(const Rect&) const;
  Rect Union(const Rect& other) const;

 private:
  RECT data_;
};

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect& rect);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect* rect);

#endif  // COMMON_WIN_RECT_H_

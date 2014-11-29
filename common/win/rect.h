// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_rect_h)
#define INCLUDE_common_win_rect_h

#include "common/common_export.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace win {

class COMMON_EXPORT Rect final {
  private: RECT data_;

  public: Rect(const Point& origin, const Point& bottom_right);
  public: Rect(const Point& origin, const Size& size);
  public: explicit Rect(const Size& size);
  public: Rect(const Rect& other);
  public: Rect(const RECT& other);
  public: Rect(int l, int t, int r, int b);
  public: Rect();
  public: ~Rect();

  public: explicit operator RECT() const;

  public: Rect& operator=(const Rect& other);
  public: Rect& operator+=(const Rect& other);
  public: bool operator==(const Rect& other) const;
  public: bool operator!=(const Rect& other) const;
  public: bool operator<(const Rect& other) const;
  public: bool operator<=(const Rect& other) const;
  public: bool operator>(const Rect& other) const;
  public: bool operator>=(const Rect& other) const;
  public: Rect operator+(const Rect& other) const { return Union(other); }
  public: Rect operator-(const Size& size) const;

  public: int area() const { return width() * height(); }
  public: int bottom() const { return data_.bottom; }
  public: Point bottom_left() const { return Point(left(), bottom()); }
  public: Point bottom_right() const { return Point(right(), bottom()); }
  public: bool empty() const { return width() <= 0 || height() <= 0; }
  public: int height() const { return bottom() - top(); }
  public: int left() const { return data_.left; }
  public: Point origin() const { return Point(left(), top()); }
  public: void set_origin(const Point& new_origin);
  public: int right() const { return data_.right; }
  public: Size size() const { return Size(width(), height()); }
  public: int top() const { return data_.top; }
  public: Point top_right() const { return Point(right(), top()); }
  public: int width() const { return right() - left(); }

  public: bool Contains(const Point& pt) const;
  public: bool Contains(const Rect& other) const;
  public: Rect Intersect(const Rect&) const;
  public: Rect Union(const Rect& other) const;
};

} // namespace win
} // namespace common

#include <ostream>

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect& rect);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect* rect);

#endif //!defined(INCLUDE_common_win_rect_h)

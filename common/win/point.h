// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_point_h)
#define INCLUDE_common_win_point_h

#include <windows.h>

#include "common/common_export.h"

namespace common {
namespace win {

class Size;

class COMMON_EXPORT Point {
  private: POINT data_;

  public: explicit Point(POINTS other);
  public: Point(const Point& other);
  public: Point(const POINT& other);
  public: Point(int x, int y);
  public: Point();

  public: operator POINT() const { return data_; }

  public: Point& operator=(const Point& other);
  public: bool operator==(const Point& other) const;
  public: bool operator!=(const Point& other) const;
  public: Point operator+(const Point& other) const;
  public: Point operator+(const Size& size) const;
  public: Point operator-(const Size& size) const;
  public: Size operator-(const Point& other) const;

  public: int x() const { return data_.x; }
  public: void set_x(int x) { data_.x = x; }
  public: int y() const { return data_.y; }
  public: void set_y(int y) { data_.y = y; }

  public: Point Offset(int delta_x, int delta_y) const;
};

} // namespace win
} // namespace common

#include <ostream>

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point);

#endif //!defined(INCLUDE_common_win_point_h)

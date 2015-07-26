// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_POINT_H_
#define COMMON_WIN_POINT_H_

#include <windows.h>

#include <ostream>

#include "common/common_export.h"

namespace common {
namespace win {

class Size;

class COMMON_EXPORT Point final {
 public:
  explicit Point(POINTS other);
  explicit Point(const POINT& other);
  Point(const Point& other);
  Point(int x, int y);
  Point();

  operator POINT() const { return data_; }
  Point& operator=(const Point& other);
  bool operator==(const Point& other) const;
  bool operator!=(const Point& other) const;
  Point operator+(const Point& other) const;
  Point operator+(const Size& size) const;
  Point operator-(const Size& size) const;
  Size operator-(const Point& other) const;

  int x() const { return data_.x; }
  void set_x(int x) { data_.x = x; }
  int y() const { return data_.y; }
  void set_y(int y) { data_.y = y; }

  Point Offset(int delta_x, int delta_y) const;

 private:
  POINT data_;
};

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point);

#endif  // COMMON_WIN_POINT_H_

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/point.h"
#include "common/win/point_ostream.h"

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point) {
  return out << "(" << point.x << "," << point.y << ")";
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point) {
  return out << *point;
}

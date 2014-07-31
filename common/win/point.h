// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_point_h)
#define INCLUDE_common_win_point_h

#include "common/common_export.h"

namespace common {
namespace win {

class Size;

struct COMMON_EXPORT Point : POINT {
  Point(int x, int y);
  Point(POINTS pt);
  Point();

 Point operator+(const Size& size) const;
 Point operator-(const Size& size) const;
 Size operator-(const Point& other) const;

 Point Offset(int delta_x, int delta_y) const;
};

} // namespace win
} // namespace common

#include <ostream>

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point& point);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Point* point);

#endif //!defined(INCLUDE_common_win_point_h)

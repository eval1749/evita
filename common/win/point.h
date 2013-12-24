// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_point_h)
#define INCLUDE_common_win_point_h

#include "common/common_export.h"

namespace common {
namespace win {

struct COMMON_EXPORT Point : POINT {
  Point() {
    x = y = 0;
  }

  Point(int x, int y) {
    this->x = x;
    this->y = y;
  }

 Point(POINTS pt) {
   x = pt.x; y = pt.y;
 }
};

#define DEBUG_POINT_FORMAT "(%d,%d)"
#define DEBUG_POINT_ARG(mp_point) \
 (mp_point).x, (mp_point).y

} // namespace win
} // namespace common

#endif //!defined(INCLUDE_common_win_point_h)

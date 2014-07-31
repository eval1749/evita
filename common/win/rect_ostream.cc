// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/point.h"
#include "common/win/point_ostream.h"
#include "common/win/rect.h"
#include "common/win/rect_ostream.h"
#include "common/win/size.h"

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect& rect) {
  return out << rect.origin() << "+" << rect.size();
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect* rect) {
  return out << *rect;
}

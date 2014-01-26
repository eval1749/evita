// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/size.h"
#include "common/win/size_ostream.h"

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size) {
  return out << size.cx << "x" << size.cy;
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size) {
  return out << *size;
}

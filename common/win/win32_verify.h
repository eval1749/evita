// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_win32_verify_h)
#define INCLUDE_common_win_win32_verify_h

#include "base/logging.h"

#define WIN32_VERIFY(expr) { \
  if (!(expr)) { \
    auto const error = ::GetLastError(); \
    LOG(0) << #expr << " error=" << error; \
  } \
}

#endif //!defined(INCLUDE_common_win_win32_verify_h)

// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_WIN32_VERIFY_H_
#define COMMON_WIN_WIN32_VERIFY_H_

#include "base/logging.h"

#define WIN32_VERIFY(expr)                   \
  {                                          \
    if (!(expr)) {                           \
      auto const error = ::GetLastError();   \
      LOG(0) << #expr << " error=" << error; \
    }                                        \
  \
}

#endif  // COMMON_WIN_WIN32_VERIFY_H_

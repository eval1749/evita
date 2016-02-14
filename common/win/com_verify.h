// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_WIN_COM_VERIFY_H_
#define COMMON_WIN_COM_VERIFY_H_

#include "base/logging.h"

namespace win {

#define COM_VERIFY_LOG(expr) \
  { DVLOG(ERROR) << "hr=" << std::hex << macro_hr << ' ' << #expr; }

#if defined(_DEBUG)
#define COM_VERIFY(expr)          \
  {                               \
    const auto macro_hr = (expr); \
    if (FAILED(macro_hr)) {       \
      COM_VERIFY_LOG(expr);       \
      NOTREACHED();               \
    }                             \
  }
#else
#define COM_VERIFY(expr)          \
  {                               \
    const auto macro_hr = (expr); \
    if (FAILED(macro_hr)) {       \
      COM_VERIFY_LOG(expr);       \
    }                             \
  }
#endif

#define COM_VERIFY2(expr, ret_expr) \
  {                                 \
    const auto macro_hr = (expr);   \
    if (FAILED(macro_hr)) {         \
      COM_VERIFY_LOG(expr);         \
      return (ret_expr);            \
    }                               \
  }

}  // namespace win

#endif  // COMMON_WIN_COM_VERIFY_H_

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_WIN_COM_VERIFY_H_
#define COMMON_WIN_COM_VERIFY_H_

#include "base/logging.h"

namespace win {

#if defined(_DEBUG)
#define COM_VERIFY(expr)                                             \
  {                                                                  \
    auto const macro_hr = (expr);                                    \
    if (FAILED(macro_hr)) {                                          \
      DVLOG(ERROR) << "hr=" << std::hex << macro_hr << " " << #expr; \
      NOTREACHED();                                                  \
    }                                                                \
  \
}
#else
#define COM_VERIFY(expr)                                             \
  {                                                                  \
    auto const macro_hr = (expr);                                    \
    if (FAILED(macro_hr))                                            \
      DVLOG(ERROR) << "hr=" << std::hex << macro_hr << " " << #expr; \
  \
}
#endif

}  // namespace win

#endif  // COMMON_WIN_COM_VERIFY_H_

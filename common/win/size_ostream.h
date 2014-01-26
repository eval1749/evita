// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_size_ostream_h)
#define INCLUDE_common_win_size_ostream_h

#include <ostream>

#include "common/common_export.h"

namespace common {
namespace win {
struct Size;
}
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size);

#endif //!defined(INCLUDE_common_win_size_ostream_h)

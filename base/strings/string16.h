// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_string16_h)
#define INCLUDE_base_string16_h

#include "base/basictypes.h"
#include <string>

namespace base {

typedef wchar_t char16;
typedef std::wstring string16;
typedef std::char_traits<wchar_t> string16_char_traits;

} // namespace base

#endif //!defined(INCLUDE_base_string16_h)

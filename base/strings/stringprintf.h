// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_strings_stringprintf_h)
#define INCLUDE_base_strings_stringprintf_h

#include "base/base_export.h"
#include "base/strings/string16.h"
#include <stdarg.h>

namespace base {

BASE_EXPORT std::string StringPrintf(const char* format, ...);
BASE_EXPORT std::string StringPrintfV(const char* format, va_list ap);
BASE_EXPORT base::string16 StringPrintf(const char16* format, ...);
BASE_EXPORT base::string16 StringPrintfV(const char16* format, va_list ap);

}  // namespace base

#endif  // !defined(INCLUDE_base_strings_stringprintf_h)

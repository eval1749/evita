// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_strings_string_util_h)
#define INCLUDE_base_strings_string_util_h

#include "base/strings/string_piece.h"

namespace base {

BASE_EXPORT bool IsStringASCII(const string16& piece);
BASE_EXPORT bool IsStringASCII(const StringPiece& piece);

} // namespace base

#endif //!defined(INCLUDE_base_strings_string_util_h)

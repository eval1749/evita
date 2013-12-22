// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_strings_utf_string_conversions_h)
#define INCLUDE_base_strings_utf_string_conversions_h

#include "base/strings/string_piece.h"

namespace base {

BASE_EXPORT string16 ASCIIToUTF16(const StringPiece& ascii);

} // namespace base

#endif //!defined(INCLUDE_base_strings_utf_string_conversions_h)

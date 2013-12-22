// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/utf_string_conversions.h"

#include "base/logging.h"
#include "base/strings/string_util.h"

namespace base {

BASE_EXPORT string16 ASCIIToUTF16(const StringPiece& ascii) {
  DCHECK(IsStringASCII(ascii));
  return std::move(string16(ascii.begin(), ascii.end()));
}

}  // namespace base

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/utf_string_conversions.h"

#include "base/logging.h"
#include "base/strings/string_util.h"

namespace base {

namespace {

template<class StringType>
bool DoIsStringASCII(const StringType& str) {
  for (size_t i = 0; i < str.length(); ++i) {
    auto const ch = static_cast<uint32>(str[i]);
    if (ch > 0x7F)
      return false;
  }
  return true;
}

} // namespace

BASE_EXPORT bool IsStringASCII(const base::string16& str) {
  return DoIsStringASCII(str);
}

BASE_EXPORT bool IsStringASCII(const base::StringPiece& piece) {
  return DoIsStringASCII(piece);
}

}  // namespace base

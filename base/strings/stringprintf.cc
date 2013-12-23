// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/stringprintf.h"

namespace base {

std::string StringPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  auto const result = StringPrintfV(format, args);
  va_end(args);
  return std::move(result);
}

std::string StringPrintfV(const char* format, va_list args) {
  char buf[1025];
  ::wvsprintfA(buf, format, args);
  return std::move(std::string(buf));
}

string16 StringPrintf(const char16* format, ...) {
  va_list args;
  va_start(args, format);
  auto const result = StringPrintfV(format, args);
  va_end(args);
  return std::move(result);
}

string16 StringPrintfV(const char16* format, va_list args) {
  char16 buf[1025];
  ::wvsprintfW(buf, format, args);
  return std::move(string16(buf));
}

}  // namespace base

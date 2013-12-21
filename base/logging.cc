// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "base/logging.h"

namespace logging {

namespace {

void __declspec(noreturn) Fail(const char* psz, ...) {
    va_list args;
    va_start(args, psz);
    char sz[1024];
    ::wvsprintfA(sz, psz, args);
    va_end(args);
    ::MessageBoxA(nullptr, sz, "Fatail Error", MB_ICONERROR);
    __debugbreak();
}

base::string16 StringFormat(const char* format, ...) {
  va_list args;
  char str8[1000];
  va_start(args, format);
  ::wvsprintfA(str8, format, args);
  va_end(args);
  base::string16 str16(::lstrlenA(str8), ' ');
  auto runner8 = &str8[0];
  for (auto& it: str16) {
    it = *runner8;
    ++runner8;
  }
  return std::move(str16);
}

} // namespace

void __declspec(noreturn) CheckFailed(
    const base::string16& a,
    const base::string16& b,
    const char* str_a,
    const char* str_b,
    const char* str_op,
    const char* filename,
    int const linenum,
    const char* fname) {
  Fail(
      "Check faield: %ls\n"
      "\n"
      "File: %s\n"
      "Line: %d\n"
      "Function: %s\n"
      "\n"
      "Actual: %s = %ls\n"
      "Expect: %s = %ls\n",
      StringFormat("%s %s %s", str_a, str_op, str_b).c_str(),
      filename,
      linenum,
      fname,
      str_a, a.c_str(),
      str_b, b.c_str());
}

void CheckTrue(
    bool value,
    const char* msg,
    const char* filename,
    int const linenum,
    const char* fname) {
  if (value) return;

  ::OutputDebugStringW(
      StringFormat("%s(%d): %s: %s\r\n", filename, linenum, fname,
                   msg).c_str());

  Fail(
      "Check failed: %s\n"
      "\n"
      "File: %s\n"
      "Line: %d\n"
      "Function: %s\n",
      msg,
      filename,
      linenum,
      fname);
}
} // namespace logging

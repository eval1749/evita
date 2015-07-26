//////////////////////////////////////////////////////////////////////////////
//
// evcl - kernel - debugger
// kernel_debugger.cpp
//
// This file is part of Evita Common Lisp.
//
// Copyright (C) 1996-2006 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_debug.cpp#1 $
//
#include <stdlib.h>

#include "regex/precomp.h"
#include "regex/regex_debug.h"

namespace Debugger {

static HANDLE g_hConOut = INVALID_HANDLE_VALUE;
static BOOL g_fConOut;

static BOOL get_conout() {
  if (!g_fConOut) {
    g_hConOut = ::CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    g_fConOut = TRUE;
  }

  return g_hConOut != INVALID_HANDLE_VALUE;
}

#if defined(_DEBUG)
//////////////////////////////////////////////////////////////////////
//
// Assert
//
void /*__fastcall*/ Assert(LPCSTR pszFileName,
                           int iLineNum,
                           LPCSTR pszFunction,
                           LPCSTR pszExpr,
                           bool fExpr) {
  if (fExpr) {
    return;
  }

  Fail(
      "Assertion failed %hs\n"
      "FileName: %hs\n"
      "Line:     %d\n"
      "Function: %hs\n",
      pszExpr, pszFileName, iLineNum, pszFunction);
}

// CanNotHappen
void /*__fastcall*/ CanNotHappen(LPCSTR pszFileName,
                                 int iLineNum,
                                 LPCSTR pszFunction) {
  Fail(
      "Can't Happen!!\n"
      "FileName: %hs\n"
      "Line:     %d\n"
      "Function: %hs\n",
      pszFileName, iLineNum, pszFunction);
}

#else
// Can_Not_Happen
void /*__fastcall*/ CanNotHappen(LPCSTR, int, LPCSTR) {
  __debugbreak();
}
#endif  // defined(_DEBUG)

//////////////////////////////////////////////////////////////////////
//
// Debugger - Fail
//
void /*__fastcall*/ Fail(LPCSTR pszFormat, ...) {
  char sz[1024];
  {
    va_list args;
    va_start(args, pszFormat);
    ::wvsprintfA(sz, pszFormat, args);
    va_end(args);
  }

  ::OutputDebugStringA(sz);

  if (get_conout()) {
    DWORD cchWritten;
    ::WriteConsoleA(g_hConOut, sz, DWORD(::lstrlenA(sz)), &cchWritten, nullptr);
  }

  if (::IsDebuggerPresent()) {
    __debugbreak();
  }

  ::MessageBoxA(nullptr, sz, "Evita Common Lisp", MB_ICONERROR | MB_TASKMODAL);

  ::ExitProcess(EXIT_FAILURE);
}

//////////////////////////////////////////////////////////////////////
//
// Debugger - Printf
//
void /*__fastcall*/ Printf(LPCSTR pszFormat, ...) {
  char szString[1024];
  {
    va_list args;
    va_start(args, pszFormat);
    ::wvsprintfA(szString, pszFormat, args);
  }

  ::OutputDebugStringA(szString);

#if 0
  if (get_conout()) {
    DWORD cchWritten;
    ::WriteConsoleW(g_hConOut, szString, ::lstrlenW(szString), &cchWritten,
                    nullptr);
  }
#endif
}

void /*__fastcall*/ PrintHeader(const char* pszFname) {
  char sz[1024];
  ::wsprintfA(sz, "%d %s: ", ::GetTickCount(), pszFname);
  ::OutputDebugStringA(sz);
}

}  // namespace Debugger

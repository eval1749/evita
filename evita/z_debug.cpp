#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Common Debug Utility
// z_debug.cpp
//
// Copyright (C) 2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/z_debug.cpp#1 $
//
#include "evita/z_debug.h"

namespace Debugger
{

void Assert(
    const char* pszFile,
    int         iLine,
    const char* pszFunc,
    const char* pszExpr,
    bool        fExpr )
{
    when (fExpr) return;
    char sz[1024];
    ::wsprintfA(sz,
        "Assertion Failed!\n"
        "  File: %hs\n"
        "  Line: %d\n"
        "  Function: %hs\n"
        "  Expression: %hs\n",
        pszFile,
        iLine,
        pszFunc,
        pszExpr );
    Fail(sz);
} // Assert

void __declspec(noreturn) CanNotHappen(
    const char* pszFile,
    int         iLine,
    const char* pszFunc )
{
    char sz[1024];
    ::wsprintfA(sz,
        "Can't happen!\n"
        "  File:     %hs\n"
        "  Line:     %d\n"
        "  Function: %hs\n",
        pszFile,
        iLine,
        pszFunc );
    Fail(sz);
} // CanNotHappen

void __declspec(noreturn) Fail(const char* psz, ...)
{
    va_list args;
    va_start(args, psz);
    char sz[1024];
    ::wvsprintfA(sz, psz, args);
    va_end(args);
    ::OutputDebugStringA(sz);
    // Since showing message box changes focus and it causes another
    // assertion failure. We want to break before showing message box.
    if (::IsDebuggerPresent())
      __debugbreak();
    ::MessageBoxA(NULL, sz, "Evita Common Lisp", MB_ICONERROR);
    __debugbreak();
} // Fail

void Printf(const char* psz, ...)
{
    va_list args;
    va_start(args, psz);
    char sz[1024];
    ::wvsprintfA(sz, psz, args);
    va_end(args);
    ::OutputDebugStringA(sz);
} // Printf

} // Debugger

#include "precomp.h"
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
#include "regex_debug.h"
#include <stdlib.h> // EXIT_FAILURE

namespace Debugger
{

static HANDLE g_hConOut = INVALID_HANDLE_VALUE;
static BOOL   g_fConOut;

static BOOL
get_conout()
{
    if (! g_fConOut)
    {
        g_hConOut = ::CreateFileW(
            L"CONOUT$",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );
        g_fConOut = TRUE;
    } // if

    return g_hConOut != INVALID_HANDLE_VALUE;
} // get_conout


#if defined(_DEBUG)
//////////////////////////////////////////////////////////////////////
//
// Assert
//
void /*__fastcall*/ Assert(
    LPCSTR  pszFileName,
    int     iLineNum,
    LPCSTR  pszFunction,
    LPCSTR  pszExpr,
    bool    fExpr )
{
    if (fExpr)
    {
        return;
    }

    Fail(
        "Assertion failed %hs\n"
        "FileName: %hs\n"
        "Line:     %d\n"
        "Function: %hs\n",
        pszExpr,
        pszFileName,
        iLineNum,
        pszFunction );
} // Assert

// CanNotHappen
void /*__fastcall*/ CanNotHappen(
    LPCSTR  pszFileName,
    int     iLineNum,
    LPCSTR  pszFunction )
{
    Fail(
        "Can't Happen!!\n"
        "FileName: %hs\n"
        "Line:     %d\n"
        "Function: %hs\n",
        pszFileName,
        iLineNum,
        pszFunction );
} // Can_Not_happen

#else
// Can_Not_Happen
void /*__fastcall*/ CanNotHappen(
    LPCSTR,
    int,
    LPCSTR)
{
    __debugbreak();
} // Can_Not_happen
#endif // defined(_DEBUG)


//////////////////////////////////////////////////////////////////////
//
// Debugger - Fail
//
void /*__fastcall*/ Fail(LPCSTR pszFormat, ...)
{
    char sz[1024];
    {
        va_list args;
        va_start(args, pszFormat);
        ::wvsprintfA(sz, pszFormat, args);
        va_end(args);
    }

    ::OutputDebugStringA(sz);

    if (get_conout())
    {
        DWORD cchWritten;
        ::WriteConsoleA(
            g_hConOut,
            sz,
            DWORD(::lstrlenA(sz)),
            &cchWritten,
            NULL );
    } // if

    if (::IsDebuggerPresent())
    {
        __debugbreak();
    }

    ::MessageBoxA(
        NULL,
        sz, 
        "Evita Common Lisp",
        MB_ICONERROR | MB_TASKMODAL );

    ::ExitProcess(EXIT_FAILURE);
} // Fail


//////////////////////////////////////////////////////////////////////
//
// Debugger - Printf
//
void /*__fastcall*/ Printf(LPCSTR pszFormat, ...)
{
    char szString[1024];
    {
        va_list args;
        va_start(args, pszFormat);
        ::wvsprintfA(szString, pszFormat, args);
    }

    ::OutputDebugStringA(szString);

    #if 0
    {
        if (get_conout())
        {
            DWORD cchWritten;
            ::WriteConsoleW(
                g_hConOut,
                szString,
                ::lstrlenW(szString),
                &cchWritten,
                NULL );
        }
    }
    #endif
} // Printf

void /*__fastcall*/ PrintHeader(const char* pszFname)
{
    char sz[1024];
    ::wsprintfA(sz, "%d %s: ", ::GetTickCount(), pszFname);
    ::OutputDebugStringA(sz);
} // PrintHeader

} // Debugger

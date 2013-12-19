//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - pre-compiled header
// precomp.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/precomp.h#2 $
//
#if !defined(INCLUDE_listener_winapp_precomp_h)
#define INCLUDE_listener_winapp_precomp_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Requires at least Windows Vista
//#define _WIN32_WINNT 0x600

#define STRICT
#define INC_OLE2
#define WIN32_LEAN_AND_MEAN
// Note: When defines ISOLATION_AWARE_ENABLE=1, prsht.inl causes compilation
// error by undefined symbol |IsolationAwarePrivatenCv|.
//#define ISOLATION_AWARE_ENABLED 1

// Disable warning in windows.h.

// warning C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
#pragma warning(disable: 4191)

// warning C4668: 'symbol' is not defined as a preprocessor macro, replacing
// with '0' for 'directives'
#pragma warning(disable: 4668)

// warning C4710: 'function' : function not inlined
#pragma warning(disable: 4710)

// warning C4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable: 4820)

// warning C6385:  invalid data: accessing <buffer name>, the readable size is
// <size1> bytes, but <size2> bytes may be read: Lines: x, y
#pragma warning(disable: 6385)
#include <windows.h>
#pragma warning(default: 4191)
//#pragma warning(default: 4820)
//#pragma warning(default: 4668)
//#pragma warning(default: 6385)

//#pragma warning(disable: 4820)
#include <ShellAPI.h>   // HDROP
//#pragma warning(default: 4820)

#include <stddef.h>     // ptrdiff_t
#include <windowsx.h>   // GET_X_LPARAM

// Theme (Visual Style)
struct _IMAGELIST;
typedef _IMAGELIST* HIMAGELIST;
#pragma warning(disable: 4191)
#include <uxtheme.h>
#pragma warning(default: 4191)

#ifndef DragQueryFile
    #error "We must include ShellAPI.h"
#endif

// undocumented SWP flags. See http://www.winehq.org.
#if !defined(SWP_NOCLIENTSIZE)
    #define SWP_NOCLIENTSIZE    0x0800
    #define SWP_NOCLIENTMOVE    0x1000
#endif // !defined(SWP_NOCLIENTSIZE)

// Windows Vista
#if ! defined(LVS_EX_TRANSPARENTBKGND)
#define LVS_EX_TRANSPARENTBKGND 0x00400000
#define LVS_EX_TRANSPARENTSHADOWTEXT 0x00800000
#endif

#define WM_SYSTIMER 0x118

// warning C4373: previous versions of the compiler did not override when
// parameters only differed by const/volatile qualifiers
#pragma warning(disable:4373)

// warning C4530: C++ exception handler used, but unwind semantics are not
// enabled. Specify /EHsc
#pragma warning(disable: 4530)

// warning C4800: 'BOOL' : forcing value to bool 'true' or 'false'
// (performance warning)
#pragma warning(disable:4800)

// C4355: 'this': used in base member initializer list:
// http://msdn.microsoft.com/en-us/library/3c594ae3.aspx
#define ALLOW_THIS_IN_INITIALIZER_LIST(expr) \
    __pragma(warning(suppress:4355)) \
    expr

typedef char int8;
typedef short int16;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef unsigned int uint32;
typedef wchar_t char16;

#define unless(mp_exp)  if (! (mp_exp))
#define when(mp_exp)    if (mp_exp)
#define NoReturn __declspec(noreturn)

//#define ASSERT(mp_expr) __assume(mp_expr)
#include "./z_debug.h"

#define foreach(mp_ty, mp_enum, mp_init) \
    for (mp_ty mp_enum(mp_init); ! mp_enum.AtEnd(); mp_enum.Next())

#define lengthof(a) ( sizeof(a) / sizeof(*(a)) )

#define DISALLOW_COPY_AND_ASSIGN(mp_type) \
  public: mp_type(const mp_type&) = delete; \
  public: void operator=(const mp_type&) = delete

// warning C6400: Using 'lstrcmpiW' to perform a case-insensitive compare to
// constant string 
#pragma warning(disable: 6246)

// warning C6246: Local declaration of 'oEnum' hides declaration of the same
// name in outer scope
#pragma warning(disable: 6400)

// ObjectInHeap
class ObjectInHeap
{
    public: void* operator new(size_t cb, HANDLE h)
        { return ::HeapAlloc(h, 0, cb); }

    private: void* operator new(size_t)
        { return NULL; }

    private: void operator delete(void*)
        {}
}; // ObjectInHeap

template<class T> void
swap(T& rx, T& ry)
{
    T temp = rx;
    rx = ry;
    ry = temp;
} // swap

#if ! defined(SUPPORT_IME)
    #define SUPPORT_IME 1
#endif // ! defined(SUPPORT_IME)

class FileTime : public FILETIME
{
    public: FileTime()
    {
        dwLowDateTime  = 0;
        dwHighDateTime = 0;
    } // FileTime

    public: FILETIME& operator =(const FILETIME& ft)
    {
        dwLowDateTime  = ft.dwLowDateTime;
        dwHighDateTime = ft.dwHighDateTime;
        return *this;
    } // opeator =

    public: int Compare(const FILETIME* p) const {
      if (dwHighDateTime - p->dwHighDateTime)
        return dwHighDateTime > p->dwHighDateTime ? 1 : -1;
      if (dwLowDateTime - p->dwLowDateTime)
        return dwLowDateTime > p->dwLowDateTime ? 1 : -1;
      return 0;
    } // Compare
}; // FileTime

//////////////////////////////////////////////////////////////////////
//
// FileHandle
//
class FileHandle
{
    public: HANDLE h;

    public: FileHandle(HANDLE hFile = INVALID_HANDLE_VALUE)
    {
        h = hFile;
    } // FileHandle

    public: ~FileHandle()
    {
        Release();
    } // ~FileHandle

    public: operator HANDLE() const { return h; }

    public: FileHandle& operator =(HANDLE hHandle)
    {
        Attach(hHandle);
        return *this;
    } // operator =

    public: void Attach(HANDLE hHandle)
    {
        Release();
        h  = hHandle;
    } // Attach

    public: HANDLE Detach()
        { HANDLE h1 = h; h = INVALID_HANDLE_VALUE; return h1; }

    public: void Release()
    {
        if (INVALID_HANDLE_VALUE != h)
        {
            ::CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
        }
    } // Release
}; // FileHandle

//////////////////////////////////////////////////////////////////////
//
// Handle
//
class Handle
{
    public: HANDLE h;

    public: Handle(HANDLE hFile = NULL)
    {
        h = hFile;
    } // Handle

    public: ~Handle()
    {
        Release();
    } // ~Handle

    public: operator HANDLE() const { return h; }

    public: Handle& operator =(HANDLE hHandle)
    {
        Attach(hHandle);
        return *this;
    } // operator =

    public: void Attach(HANDLE hHandle)
    {
        Release();
        h  = hHandle;
    } // Attach

    public: HANDLE Detach()
        { HANDLE h1 = h; h = NULL; return h1; }

    public: void Release()
    {
        if (NULL != h)
        {
            ::CloseHandle(h);
            h = NULL;
        }
    } // Release
}; // Handle

//////////////////////////////////////////////////////////////////////
//
// RegKey
//  Smart handle of HKEY.
//
class RegKey
{
    public: HKEY h;

    public: RegKey() : h(NULL) {}

    public: ~RegKey()
    {
        if (NULL != h)
        {
            ::RegCloseKey(h);
        }
    } // ~RegKey

    public: operator HKEY() const { return h; }
}; // RegKey

#include "./ed_defs.h"

typedef Edit::Count Count;
typedef Edit::Posn  Posn;

#if defined(NDEBUG)
    #if defined (_M_IX86)
        #pragma function(memcpy)
        //#pragma function(memset)
    #endif // defined (_M_IX86)
    extern "C" void* myCopyMemory(void*, const void*, size_t);
    extern "C" void* myMoveMemory(void*, const void*, size_t);
    extern "C" void* myZeroMemory(void*, size_t);
#else
    #define myCopyMemory(d, s, n) ::CopyMemory(d, s, n)
    #define myMoveMemory(d, s, n) ::MoveMemory(d, s, n)
    #define myZeroMemory(d, n)    ::ZeroMemory(d, n)

#endif // NDEBUG

char16* lstrchrW(const char16*, char16);
char16* lstrrchrW(const char16*, char16);

#define DEBUG_DESTROY _DEBUG

#if defined(_DEBUG)
#define _ITERATOR_DEBUG_LEVEL 2
#endif

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not explicitly handled by a case label
#pragma warning(disable: 4061)

// warning C4062: enumerator 'identifier' in switch of enum 'enumeration' is
// not handled
// e.g. We don't want to have |case State_Limit|, |cast Kind_Max|, etc.
#pragma warning(disable: 4062)

// TODO: We should not disable warning C4350.
// warning C4350: behavior change: 'member1' called instead of 'member2'
// An rvalue cannot be bound to a non-const reference. In previous versions
// of Visual C++, it was possible to bind an rvalue to a non-const reference
// in a direct initialization. This code now gives a warning.
#pragma warning(disable: 4350)

// warning C4711: function 'function' selected for inline expansion
#pragma warning(disable: 4711)

// warning: C4640 'instance' : construction of local static object is not
// thread-safe
#define DEFINE_STATIC_LOCAL(mp_type, mp_name, ...) \
  __pragma(warning(suppress: 4640)) \
  static mp_type mp_name __VA_ARGS__

#endif //!defined(INCLUDE_listener_winapp_precomp_h)

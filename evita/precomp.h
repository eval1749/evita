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
#ifndef EVITA_PRECOMP_H_
#define EVITA_PRECOMP_H_

#if _MSC_VER > 1000
#pragma once
#endif

// See \Program Files(x86)\Windows Kit\8.1\include\shared\sdkddkver.h for
// actual value of _WIN32_WINNT_WIN8(0x602), _WIN32_WINNT_WINBLUE(0x603)
#ifndef EVITA_PRECOMP_H_
#define _WIN32_WINNT 0x0700
#endif

#ifndef EVITA_PRECOMP_H_
#define STRICT
#endif
#define INC_OLE2
#ifndef EVITA_PRECOMP_H_
#define WIN32_LEAN_AND_MEAN
#endif

// Disable warning in windows.h.

// warning C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
#pragma warning(disable : 4191)

// warning C4668: 'symbol' is not defined as a preprocessor macro, replacing
// with '0' for 'directives'
#pragma warning(disable : 4668)

// warning C4710: 'function' : function not inlined
#pragma warning(disable : 4710)

// warning C4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable : 4820)

// warning C6385:  invalid data: accessing <buffer name>, the readable size is
// <size1> bytes, but <size2> bytes may be read: Lines: x, y
#pragma warning(disable : 6385)
#include <windows.h>
#pragma warning(default : 4191)

#include <ShellAPI.h>  // HDROP

#include <stddef.h>  // ptrdiff_t
#include <stdint.h>
#include <windowsx.h>  // GET_X_LPARAM

// Theme (Visual Style)
struct _IMAGELIST;
typedef _IMAGELIST* HIMAGELIST;
#pragma warning(disable : 4191)
#include <uxtheme.h>
#pragma warning(default : 4191)

#ifndef EVITA_PRECOMP_H_
#error "We must include ShellAPI.h"
#endif

// undocumented SWP flags. See http://www.winehq.org.
#if !defined(SWP_NOCLIENTSIZE)
#define SWP_NOCLIENTSIZE 0x0800
#define SWP_NOCLIENTMOVE 0x1000
#endif

// Windows Vista
#if !defined(LVS_EX_TRANSPARENTBKGND)
#define LVS_EX_TRANSPARENTBKGND 0x00400000
#define LVS_EX_TRANSPARENTSHADOWTEXT 0x00800000
#endif

#define WM_SYSTIMER 0x118

// warning C4373: previous versions of the compiler did not override when
// parameters only differed by const/volatile qualifiers
#pragma warning(disable : 4373)

// warning C4800: 'BOOL' : forcing value to bool 'true' or 'false'
// (performance warning)
#pragma warning(disable : 4800)

#include "base/basictypes.h"

// TODO(yosi): We should not include string16.h in precomp.h. This is for
// |char16|.
#include "base/strings/string16.h"
typedef base::char16 char16;

#include "evita/z_debug.h"

// warning C6400: Using 'lstrcmpiW' to perform a case-insensitive compare to
// constant string
#pragma warning(disable : 6246)

// warning C6246: Local declaration of 'oEnum' hides declaration of the same
// name in outer scope
#pragma warning(disable : 6400)

// ObjectInHeap
class ObjectInHeap final {
 public:
  void* operator new(size_t cb, HANDLE h) { return ::HeapAlloc(h, 0, cb); }

  void* operator new(size_t) = delete;
  void operator delete(void*) = delete;
};

//////////////////////////////////////////////////////////////////////
//
// FileHandle
//
class FileHandle final {
  HANDLE h;

  explicit FileHandle(HANDLE hFile = INVALID_HANDLE_VALUE) { h = hFile; }

  ~FileHandle() { Release(); }

  operator HANDLE() const { return h; }

  FileHandle& operator=(HANDLE hHandle) {
    Attach(hHandle);
    return *this;
  }

  void Attach(HANDLE hHandle) {
    Release();
    h = hHandle;
  }

  HANDLE Detach() {
    HANDLE h1 = h;
    h = INVALID_HANDLE_VALUE;
    return h1;
  }

  void Release() {
    if (h == INVALID_HANDLE_VALUE)
      return;
    ::CloseHandle(h);
    h = INVALID_HANDLE_VALUE;
  }
};

//////////////////////////////////////////////////////////////////////
//
// Handle
//
class Handle final {
  HANDLE h;

  explicit Handle(HANDLE hFile = nullptr) : h(hFile) {}

  ~Handle() { Release(); }

  operator HANDLE() const { return h; }

  Handle& operator=(HANDLE hHandle) {
    Attach(hHandle);
    return *this;
  }

  void Attach(HANDLE hHandle) {
    Release();
    h = hHandle;
  }

  HANDLE Detach() {
    HANDLE h1 = h;
    h = nullptr;
    return h1;
  }

  void Release() {
    if (h == nullptr)
      return;
    ::CloseHandle(h);
    h = nullptr;
  }
};

//////////////////////////////////////////////////////////////////////
//
// RegKey
//  Smart handle of HKEY.
//
struct RegKey final {
  HKEY h;

  RegKey() : h(nullptr) {}

  ~RegKey() {
    if (h)
      ::RegCloseKey(h);
  }

  operator HKEY() const { return h; }
};

#include "evita/ed_defs.h"

#define myCopyMemory(d, s, n) ::CopyMemory(d, s, n)
#define myMoveMemory(d, s, n) ::MoveMemory(d, s, n)

char16* lstrchrW(const char16*, char16);
char16* lstrrchrW(const char16*, char16);

#if defined(_DEBUG)
#define _ITERATOR_DEBUG_LEVEL 2
#endif

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not explicitly handled by a case label
#pragma warning(disable : 4061)

// warning C4062: enumerator 'identifier' in switch of enum 'enumeration' is
// not handled
// e.g. We don't want to have |case State_Limit|, |cast Kind_Max|, etc.
#pragma warning(disable : 4062)

// warning C4127: conditional expression is constant
// For DCHECK_XX(x)
#pragma warning(disable : 4127)

// warning: C4640 'instance' : construction of local static object is not
// thread-safe
#define DEFINE_STATIC_LOCAL(mp_type, mp_name, ...)             \
  __pragma(warning(suppress : 4640)) static mp_type& mp_name = \
      *new mp_type __VA_ARGS__

// Note: base::Bind generats C4191.
// C4191: 'operator/operation' : unsafe conversion from 'type of expression'
// to 'type required'
#pragma warning(disable : 4191)

#endif  // EVITA_PRECOMP_H_

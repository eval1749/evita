//////////////////////////////////////////////////////////////////////////////
//
// Regex - Pre-Compiled Header
// precomp.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/precomp.h#1 $
//
#if !defined(INCLUDE_regex_precomp_h)
#define INCLUDE_regex_precomp_h

#pragma once

#pragma warning(disable: 4481)
#pragma warning(disable: 4627)
#pragma warning(disable: 4668)
#pragma warning(disable: 4820)

// warning C4710: 'function' : function not inlined
#pragma warning(disable: 4710)

// warning C4711: function 'function' selected for inline expansion
#pragma warning(disable: 4711)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef char int8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef unsigned int uint32;
typedef wchar_t char16;
typedef intptr_t Int;

#include "../regex/regex_defs.h"

#if NDEBUG && 0

#undef CopyMemory
#define CopyMemory(d, s, c) myCopyMemory(d, s, c)

#undef FillMemory
#define FillMemory(d, c, k) myFillMemory(d, c, k)

//#undef ZeroMemory
//#define ZeroMemory(d, c) myZeroMemory(d, c)

extern "C"
{
    void* __fastcall myCopyMemory(void* dst, const void* src, size_t count);
    void* __fastcall myFillMemory(void* dst, size_t count, BYTE);
    void* __fastcall myZeroMemory(void* dst, size_t count);
} // extern "C"

#pragma function(memcmp)
#if _M_IX86
    #pragma function(memcpy)
    #pragma function(memset)
#endif // _M_IX86

#define my_memcmp     memcmp
#define my_memcpy     memcpy
#define my_memmove    memmove
#define my_memset     memset


#endif // NDEBUG

// C6246: Local declaration of 'foo' hides declaration of the same name in outer scope
#pragma warning(disable: 6246)

#define foreach(mp_enum, mp_var, mp_arg) \
    for (mp_enum mp_var(mp_arg); ! (mp_var).AtEnd(); (mp_var).Next())

#define unless(mp_expr) if (! (mp_expr) )

#define DISALLOW_COPY_AND_ASSIGN(mp_type) \
  private: mp_type(const mp_type&); \
  private: void operator=(const mp_type&)

#include "evita/z_debug.h"

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not explicitly handled by a case label
#pragma warning(disable: 4061)

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not handled
#pragma warning(disable: 4062)

// warning C4302: 'conversion' : truncation from 'type 1' to 'type 2'
#pragma warning(disable: 4302)

// warning C4365: 'action' : conversion from 'type_1' to 'type_2',
// signed/unsigned mismatch
#pragma warning(disable: 4365)

// warning C4530: C++ exception handler used, but unwind semantics are not
// enabled. Specify /EHsc
#pragma warning(disable: 4530)

// warning C4625: 'derived class' : copy constructor could not be generated
// because a base class copy constructor is inaccessible
#pragma warning(disable: 4625)

// warning C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(disable: 4626)

// warning C4946: reinterpret_cast used between related classes: 
// 'class1' and 'class2'
#pragma warning(disable: 4946)


#endif // !defined(INCLUDE_regex_precomp_h)

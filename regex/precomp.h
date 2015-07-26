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
#ifndef REGEX_PRECOMP_H_
#define REGEX_PRECOMP_H_

#pragma once

#pragma warning(disable : 4481)
#pragma warning(disable : 4627)
#pragma warning(disable : 4668)
#pragma warning(disable : 4820)

// warning C4710: 'function' : function not inlined
#pragma warning(disable : 4710)

// warning C4711: function 'function' selected for inline expansion
#pragma warning(disable : 4711)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdint.h>
#include <windows.h>

// typedef char int8;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef unsigned int uint;
typedef uint32_t uint32;
typedef wchar_t char16;
typedef intptr_t Int;

#include "regex/regex_defs.h"

// C6246: Local declaration of 'foo' hides declaration of the same name in outer
// scope
#pragma warning(disable : 6246)

#define foreach(mp_enum, mp_var, mp_arg) \
  for (mp_enum mp_var(mp_arg); !(mp_var).AtEnd(); (mp_var).Next())

#define unless(mp_expr) if (!(mp_expr))

#include "../evita/z_debug.h"

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not explicitly handled by a case label
#pragma warning(disable : 4061)

// warning C4061: enumerator 'identifier' in switch of enum 'enumeration' is
// not handled
#pragma warning(disable : 4062)

// warning C4302: 'conversion' : truncation from 'type 1' to 'type 2'
#pragma warning(disable : 4302)

// warning C4365: 'action' : conversion from 'type_1' to 'type_2',
// signed/unsigned mismatch
#pragma warning(disable : 4365)

// warning C4530: C++ exception handler used, but unwind semantics are not
// enabled. Specify /EHsc
#pragma warning(disable : 4530)

// warning C4625: 'derived class' : copy constructor could not be generated
// because a base class copy constructor is inaccessible
#pragma warning(disable : 4625)

// warning C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(disable : 4626)

// warning C4946: reinterpret_cast used between related classes:
// 'class1' and 'class2'
#pragma warning(disable : 4946)

#endif  // REGEX_PRECOMP_H_

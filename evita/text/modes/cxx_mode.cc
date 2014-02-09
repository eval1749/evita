// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/cxx_mode.h"

#include "evita/text/modes/clike_lexer.h"
#include "evita/text/modes/mode.h"

#define DEBUG_LEXER 0

namespace text {
namespace {
//////////////////////////////////////////////////////////////////////
//
// C Keywords
//
static const char16* k_rgpwszCKeyword[] = {
  // Operator
  L"+", L"-", L"*", L"/", L"%", L"&", L"^", L"|",
  L"+=", L"-=", L"*=", L"/=", L"%=", L"&=", L"^=", L"|=",
  L"&&", L"||",
  L"++", L"--",
  L"<<", L">>", L"<<=", L">>=",
  L"==", L"!=", L"<", L"<=", L">", L">=",
  L"!", L"~",
  L"(", L")", L"[", L"]", L"{", L"}",

  L"asm",
  L"auto",
  L"break",
  L"case",
  L"char",
  L"const",
  L"continue",
  L"decltype",
  L"default",
  L"do",
  L"double",
  L"else",
  L"enum",
  L"extern",
  L"float",
  L"for",
  L"goto",
  L"if",
  L"int",
  L"long",
  L"mutable",
  L"nullptr",
  L"register",
  L"return",
  L"short",
  L"signed",
  L"sizeof",
  L"static",
  L"static_assert",
  L"struct",
  L"switch",
  L"typedef",
  L"union",
  L"unsigned",
  L"void",
  L"volatile",
  L"while",

  // CPP
  // TODO yosi@msn.com 2008-06-14 How do we support CPP operators, such as
  // defined, ##, #, #@?
  // TODO yosi@msn.com 2008-06-14 How do we support #pragma operators, such
  // as #pragma warning, comment, and so on.
  L"#define",
  L"#else",
  L"#elif",
  L"#endif",
  L"#error",
  L"#if",
  L"#ifdef",
  L"#ifndef",
  L"#import", // MSC
  L"#include",
  L"#line",
  L"#pragma",
  L"#undef",
  L"#using", // MSC

  // ANSI-C Predefined Macros
  L"__DATE__",
  L"__FILE__",
  L"__LINE__",
  L"__STDC__",
  L"__TIME__",
  L"__TIMESTAMP__",

  // MSC Predefined Macros
  L"_ATL_VER",
  L"_CHAR_UNSIGNED",
  L"__CLR_VER",
  L"__cplusplus_cli",
  L"__COUNTER__",
  L"__cplusplus",
  L"__CPPLIB_VER",
  L"__CPPRTTI",
  L"__CPPUNWIND",
  L"__DEBUG",
  L"_DLL",
  L"__FUNCDNAME__",
  L"__FUNCSIG__",
  L"__FUNCTION__",
  L"_INTEGRAL_MAX_BITS",
  L"_M_ALPHA",
  L"_M_CEE",
  L"_M_CEE_PURE",
  L"_M_CEE_SAFE",
  L"_M_IX86",
  L"_MIA64",
  L"_M_IX86_FP",
  L"_M_MPPC",
  L"_M_MRX000",
  L"_M_PPC",
  L"_M_X64",
  L"_MANAGED",
  L"_MFC_VER",
  L"_MSC_EXTENSIONS",
  L"_MSC_VER",
  L"_MSVC_RUNTIME_CHECKES",
  L"_MT",
  L"_NATIVE_WCHAR_T_DEFINED",
  L"_OPENMP",
  L"_VC_NODEFAULTLIB",
  L"_WCHAR_T_DEFINED",
  L"_WIN32",
  L"_WIN64",
  L"_Wp64",
};

//////////////////////////////////////////////////////////////////////
//
// C++ Keywords
//
static const char16* k_rgpwszCxxKeyword[] = {
  L"abstract", // C#
  // L"and",
  // L"and_eq",
  // L"asm", // C
  // L"auto", // C
  // L"bitand",
  // L"bitor",
  L"bool",
  // L"break", // C
  // L"case", // C
  L"catch",
  // L"char", // C
  L"checked", // C#
  L"class",
  // L"compl",
  // L"const", // C
  L"const_cast",
  // L"continue", // C
  // L"default", // C
  L"delete",
  L"delegate", // C#
  // L"do", // C
  // L"double", // C
  L"dynamic_cast",
  // L"else", // C
  // L"enum", // C
  L"explicit",
  // L"export", // C
  // L"extern", // C
  L"false",
  // L"float", // C
  // L"for", // C
  L"friend",
  // L"goto", // C
  // L"if", // C
  L"implicit", // C#
  L"inline",
  // L"int", // C
  L"namespace",
  L"new",
  // L"not", // C
  // L"not_eq", // C
  L"operator",
  // L"or", // C
  // L"or_eq", // C
  L"private",
  L"protected",
  L"public",
  // L"register", // C
  L"reinterpret_cast",
  // L"return", // C
  // L"short", // C
  // L"signed", // C
  // L"sizeof", // C
  // L"static", // C
  L"static_cast",
  // L"struct", // C
  // L"switch", // C
  L"template",
  L"this",
  L"throw",
  L"true",
  L"try",
  // L"typedef", // C
  L"typeid",
  L"typename",
  // L"union", // C
  // L"unsigned", // C
  L"using",
  L"var", // C# continue keyword
  // L"void", // C
  L"virtual",
  // L"volatile",
  L"wchar_t",
  L"where", // C# contextual keyword
  // L"while",
  // L"xor",
  // L"xor_eq",

  // Microsoft C++
  L"__abstract",
  L"__alignof",
  L"__asm",
  L"__assume",
  L"__based",
  L"__box",
  L"__cdecl",
  L"__declspec",
  //L"__delegate",
  //L"delegate", context sensitive keyword
  L"deprecated",
  L"dllexport",
  L"dllimport",
  //L"event",
  //L"__event",
  L"__except",
  L"__fastcall",
  L"__finally",
  L"__forceinline",
  L"generic",
  L"__inline",
  L"__int8",
  L"__int16",
  L"__int32",
  L"__int64",
  L"__interface",
  L"__Leave",
  L"__m64",
  L"__m128",
  L"__m128d",
  L"__m128i",
  L"__pragma",
  L"__raise",
  L"__sealed",
  L"sealed",
  L"selectany",
  L"__stdcall",
  L"__super",
  L"__unaligned",
  L"__unhook",
  L"__uuidof",
  L"__value",
  L"__w64",

  // Standard C Library
  L"NULL",
  L"_complex",
  L"div_t",
  L"ldiv_t",
  L"ptrdiff_t",
  L"size_t",
  L"va_arg",
  L"va_end",
  L"va_list",
  L"va_start",
  L"wchar_t",

  // Win32
  L"HANDLE",
  L"HKEY",
  L"INVALID_HANDLE_VALUE",

  // Vogue Extensions
  L"char16",
  L"foreach",
  L"interface",
  L"arraysize",
  L"override",
  L"uint",
  L"uint32",
  L"uint64",
  L"uint8",
  L"unless",
  L"when",
};

//////////////////////////////////////////////////////////////////////
//
// CxxKeywordTable
//
class CxxKeywordTable : public common::Singleton<CxxKeywordTable>,
                        public NewLexer::KeywordTable {
  DECLARE_SINGLETON_CLASS(CxxKeywordTable);

  private: CxxKeywordTable() {
    AddKeywords(k_rgpwszCKeyword, arraysize(k_rgpwszCKeyword));
    AddKeywords(k_rgpwszCxxKeyword, arraysize(k_rgpwszCxxKeyword));
  }

  public: ~CxxKeywordTable() = default;

  DISALLOW_COPY_AND_ASSIGN(CxxKeywordTable);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CxxLexer
//
class CxxLexer : public ClikeLexer {
  public: CxxLexer(Buffer* pBuffer)
    : ClikeLexer(pBuffer, CxxKeywordTable::instance(), '#') {
  }

  DISALLOW_COPY_AND_ASSIGN(CxxLexer);
};

//////////////////////////////////////////////////////////////////////
//
// CxxMode
//
class CxxMode : public Mode {
  private: CxxLexer m_oLexer;

  // ctor/dtor
  public: CxxMode(ModeFactory* pFactory, Buffer* pBuffer)
      : Mode(pFactory, pBuffer), m_oLexer(pBuffer) {
  }

  public: ~CxxMode() = default;

  // [D]
  public: virtual bool DoColor(Count lCount) override {
    return m_oLexer.Run(lCount);
  }

  DISALLOW_COPY_AND_ASSIGN(CxxMode);
};

//////////////////////////////////////////////////////////////////////
//
// CxxModeFactory
//
CxxModeFactory::CxxModeFactory() {
}

CxxModeFactory::~CxxModeFactory() {
}

Mode* CxxModeFactory::Create(Buffer* pBuffer) {
  return new CxxMode(this, pBuffer);
}

} // namespace text

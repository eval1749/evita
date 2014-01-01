#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - C++ Mode
// listener/winapp/mode_Cxx.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Cxx.cpp#17 $
//
#include "./mode_Cxx.h"

#define DEBUG_LEXER 0

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// C Keywords
//
static const char16*
k_rgpwszCKeyword[] =
{
    // Operator
    L"+",  L"-",  L"*",  L"/",  L"%",  L"&",  L"^",  L"|",
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
    //  defined, ##, #, #@?
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
    L"#import",         // MSC
    L"#include",
    L"#line",
    L"#pragma",
    L"#undef",
    L"#using",          // MSC

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
}; // k_rgpwszCKeyword

//////////////////////////////////////////////////////////////////////
//
// C++ Keywords
//
static const char16*
k_rgpwszCxxKeyword[] = {
    L"abstract",    // C#
    // L"and",
    // L"and_eq",
    // L"asm",      // C
    // L"auto",     // C
    // L"bitand",
    // L"bitor",
    L"bool",
    // L"break",    // C
    // L"case",     // C
    L"catch",
    // L"char",     // C
    L"checked",     // C#
    L"class",
    // L"compl",
    // L"const",    // C
    L"const_cast",
    // L"continue", // C
    // L"default",  // C
    L"delete",
    L"delegate",    // C#
    // L"do",       // C
    // L"double",   // C
    L"dynamic_cast",
    // L"else",     // C
    // L"enum",     // C
    L"explicit",
    // L"export",   // C
    // L"extern",   // C
    L"false",
    // L"float",    // C
    // L"for",      // C
    L"friend",
    // L"goto",     // C
    // L"if",       // C
    L"implicit",    // C#
    L"inline",
    // L"int",      // C
    L"namespace",
    L"new",
    // L"not",      // C
    // L"not_eq",   // C
    L"operator",
    // L"or",       // C
    // L"or_eq",    // C
    L"private",
    L"protected",
    L"public",
    // L"register", // C
    L"reinterpret_cast",
    // L"return",   // C
    // L"short",    // C
    // L"signed",   // C
    // L"sizeof",   // C
    // L"static",   // C
    L"static_cast",
    // L"struct",   // C
    // L"switch",   // C
    L"template",
    L"this",
    L"throw",
    L"true",
    L"try",
    // L"typedef",  // C
    L"typeid",
    L"typename",
    // L"union",    // C
    // L"unsigned", // C
    L"using",
    L"var",         // C# continue keyword
    // L"void",     // C
    L"virtual",
    // L"volatile",
    L"wchar_t",
    L"where",       // C# contextual keyword
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
    //L"delegate",  context sensitive keyword
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
    L"lengthof",
    L"override",
    L"uint",
    L"uint32",
    L"uint64",
    L"uint8",
    L"unless",
    L"when",
}; // k_rgpwszCxxKeyword

//////////////////////////////////////////////////////////////////////
//
// Java Keywords
//
static const char16*
k_rgpwszJavaKeyword[] =
{

    // Operator
    L"+",  L"-",  L"*",  L"/",  L"%",  L"&",  L"^",  L"|",
    L"+=", L"-=", L"*=", L"/=", L"%=", L"&=", L"^=", L"|=",
    L"&&", L"||",
    L"++", L"--",
    L"<<", L">>", L"<<=", L">>=",
    L"==", L"!=", L"<", L"<=", L">", L">=",
    L"!", L"~",
    L"(", L")", L"[", L"]", L"{", L"}",

    L"abstract",
    L"assert",       // 3rd
    L"boolean",
    L"break",
    L"byte",
    L"case",
    L"catch",
    L"char",
    L"class",
    L"const",
    L"continue",
    L"default",
    L"do",
    L"double",
    L"else",
    L"enum",         // 3rd
    L"extends",
    L"final",
    L"finally",
    L"float",
    L"for",
    L"if",
    L"goto",
    L"implements",
    L"import",
    L"instanceof",
    L"int",
    L"interface",
    L"long",
    L"native",
    L"new",
    L"package",
    L"private",
    L"protected",
    L"public",
    L"return",
    L"short",
    L"static",
    L"strictfp",
    L"super",
    L"switch",
    L"synchronized",
    L"this",
    L"throw",
    L"throws",
    L"transient",
    L"try",
    L"void",
    L"volatile",
    L"while",

    // Literals
    L"false",
    L"null",
    L"true",

    // Interfaces in java.lang
    L"Appendable",
    L"CharSequence",
    L"Cloneable",
    L"Comparable",
    L"Iterable",
    L"Readable",
    L"Runnable",
    L"Thread.UncaughtExceptionHandler",

    // Classes in java.lang
    L"Boolean",
    L"Byte",
    L"Character",
    L"Character.Subset",
    L"Character.Unicode",
    L"Class",
    L"ClassLoader",
    L"Compiler",
    L"Double",
    L"Enum",
    L"Float",
    L"InheritableThreadLocal",
    L"Integer",
    L"Long",
    L"Math",
    L"Number",
    L"Object",
    L"Package",
    L"Process",
    L"ProcessBuilder",
    L"Runtime",
    L"RuntimePermission",
    L"SecurityManager",
    L"Short",
    L"StackTraceElement",
    L"StrictMath",
    L"String",
    L"StringBuffer",
    L"StringBuilder",
    L"System",
    L"Thread",
    L"ThreadGroup",
    L"ThreadLocal",
    L"Throwable",
    L"Void",

    // Exceptions in java.lang
    L"ArithmeticException",
    L"ArrayIndexOutOfBoundsException",
    L"ArrayStoreException",
    L"ClassCastException",
    L"ClassNotFoundException",
    L"CloneNotSupportedException",
    L"EnumConstantNotPresentException",
    L"Exception",
    L"IllegalAccessException",
    L"IllegalArgumentException",
    L"IllegalMonitorStateException",
    L"IllegalStateException",
    L"IllegalThreadStateException",
    L"IndexOutOfBoundsException",
    L"InstantiationException",
    L"InterruptedException",
    L"NegativeArraySizeException",
    L"NoSuchFieldException",
    L"NoSuchMethodException",
    L"NullPointerException",
    L"NumberFormatException",
    L"RuntimeException",
    L"SecurityException",
    L"StringIndexOutOfBoundsException",
    L"TypeNotPresentException",
    L"UnsupportedOperationException",

    // Errors in java.lang
    L"AbstractMethodError",
    L"AssertionError",
    L"ClassCircularityError",
    L"ClassFormatError",
    L"Error",
    L"ExceptionInInitializerError",
    L"IllegalAccessError",
    L"IncompatibleClassChangeError",
    L"InstantiationError",
    L"InternalError",
    L"LinkageError",
    L"NoClassDefFoundError",
    L"NoSuchFieldError",
    L"NoSuchMethodError",
    L"OutOfMemoryError",
    L"StackOverflowError",
    L"ThreadDeath",
    L"UnknownError",
    L"UnsatisfiedLinkError",
    L"UnsupportedClassVersionError",
    L"VerifyError",
    L"VirtualMachineError",

    // Methods of java.lang.Object
    #if 0
        L".clone",
        L".equals",
        L".finalize",
        L".getClass",
        L".hashCode",
        L".notify",
        L".notifyAll",
        L".toString",
        L".wait",
    #endif

    // Annotations
    L"@Deprecated",
    L"@Override",
    L"@SuppressWarnings",
}; // k_rgpwszJavaKeyword

// Cxx mode character syntax
static const uint
k_rgnCxxCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),   // 0x22 "
    CharSyntax::Syntax_Punctuation,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_StringQuote | (0x27 << CharSyntax::Trait_PairShift),   // 0x27 '
    CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),   // 0x28 (
    CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),   // 0x29 )
    CharSyntax::Syntax_Punctuation,                 // 0x2A *
    CharSyntax::Syntax_Punctuation,                 // 0x2B +
    CharSyntax::Syntax_Punctuation,                 // 0x2C ,
    CharSyntax::Syntax_Punctuation,                 // 0x2D -
    CharSyntax::Syntax_Punctuation,                 // 0x2E .
    CharSyntax::Syntax_Punctuation,                 // 0x2F /

    CharSyntax::Syntax_Word,                        // 0x30 0
    CharSyntax::Syntax_Word,                        // 0x31 1
    CharSyntax::Syntax_Word,                        // 0x32 2
    CharSyntax::Syntax_Word,                        // 0x33 3
    CharSyntax::Syntax_Word,                        // 0x34 4
    CharSyntax::Syntax_Word,                        // 0x35 5
    CharSyntax::Syntax_Word,                        // 0x36 6
    CharSyntax::Syntax_Word,                        // 0x37 7
    CharSyntax::Syntax_Word,                        // 0x38 8
    CharSyntax::Syntax_Word,                        // 0x39 9
    CharSyntax::Syntax_Punctuation,                 // 0x3A :
    CharSyntax::Syntax_Punctuation,                 // 0x3B ;
    CharSyntax::Syntax_Punctuation,                 // 0x3C <
    CharSyntax::Syntax_Punctuation,                 // 0x3D =
    CharSyntax::Syntax_Punctuation,                 // 0x3E >
    CharSyntax::Syntax_Punctuation,                 // 0x3F ?

    CharSyntax::Syntax_Punctuation,                 // 0x40 @
    CharSyntax::Syntax_Word,                        // 0x41 A
    CharSyntax::Syntax_Word,                        // 0x42 B
    CharSyntax::Syntax_Word,                        // 0x43 C
    CharSyntax::Syntax_Word,                        // 0x44 D
    CharSyntax::Syntax_Word,                        // 0x45 E
    CharSyntax::Syntax_Word,                        // 0x46 F
    CharSyntax::Syntax_Word,                        // 0x47 G
    CharSyntax::Syntax_Word,                        // 0x48 H
    CharSyntax::Syntax_Word,                        // 0x49 I
    CharSyntax::Syntax_Word,                        // 0x4A J
    CharSyntax::Syntax_Word,                        // 0x4B K
    CharSyntax::Syntax_Word,                        // 0x4C L
    CharSyntax::Syntax_Word,                        // 0x4D M
    CharSyntax::Syntax_Word,                        // 0x4E N
    CharSyntax::Syntax_Word,                        // 0x4F O

    CharSyntax::Syntax_Word,                        // 0x50 P
    CharSyntax::Syntax_Word,                        // 0x51 Q
    CharSyntax::Syntax_Word,                        // 0x52 R
    CharSyntax::Syntax_Word,                        // 0x53 S
    CharSyntax::Syntax_Word,                        // 0x54 T
    CharSyntax::Syntax_Word,                        // 0x55 U
    CharSyntax::Syntax_Word,                        // 0x56 V
    CharSyntax::Syntax_Word,                        // 0x57 W
    CharSyntax::Syntax_Word,                        // 0x58 X
    CharSyntax::Syntax_Word,                        // 0x59 Y
    CharSyntax::Syntax_Word,                        // 0x5A Z
    CharSyntax::Syntax_OpenParen  | (0x5D << CharSyntax::Trait_PairShift),    // 0x5B [
    CharSyntax::Syntax_Escape,                      // 0x5C backslash(\)
    CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),    // 0x5D ]
    CharSyntax::Syntax_Punctuation,                 // 0x5E ^
    CharSyntax::Syntax_Word,                        // 0x5F _

    CharSyntax::Syntax_Punctuation,                 // 0x60 `
    CharSyntax::Syntax_Word,                        // 0x61 a
    CharSyntax::Syntax_Word,                        // 0x62 b
    CharSyntax::Syntax_Word,                        // 0x63 c
    CharSyntax::Syntax_Word,                        // 0x64 d
    CharSyntax::Syntax_Word,                        // 0x65 e
    CharSyntax::Syntax_Word,                        // 0x66 f
    CharSyntax::Syntax_Word,                        // 0x67 g
    CharSyntax::Syntax_Word,                        // 0x68 h
    CharSyntax::Syntax_Word,                        // 0x69 i
    CharSyntax::Syntax_Word,                        // 0x6A j
    CharSyntax::Syntax_Word,                        // 0x6B k
    CharSyntax::Syntax_Word,                        // 0x6C l
    CharSyntax::Syntax_Word,                        // 0x6D m
    CharSyntax::Syntax_Word,                        // 0x6E n
    CharSyntax::Syntax_Word,                        // 0x6F o

    CharSyntax::Syntax_Word,                        // 0x70 p
    CharSyntax::Syntax_Word,                        // 0x71 q
    CharSyntax::Syntax_Word,                        // 0x72 r
    CharSyntax::Syntax_Word,                        // 0x73 s
    CharSyntax::Syntax_Word,                        // 0x74 t
    CharSyntax::Syntax_Word,                        // 0x75 u
    CharSyntax::Syntax_Word,                        // 0x76 v
    CharSyntax::Syntax_Word,                        // 0x77 w
    CharSyntax::Syntax_Word,                        // 0x78 x
    CharSyntax::Syntax_Word,                        // 0x79 y
    CharSyntax::Syntax_Word,                        // 0x7A z
    CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),    // 0x7B {
    CharSyntax::Syntax_Word,                        // 0x7C |
    CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),    // 0x7D }
    CharSyntax::Syntax_Word,                        // 0x7E ^
    CharSyntax::Syntax_Control,                     // 0x7F DEL
}; // k_rgnCxxCharSyntax

/// <summary>
///   A base class of C-like language.
/// </summary>
class ClikeLexer : public NewLexer::LexerBase
{
    public: enum State
    {
        State_StartLine,

        State_Annotation,

        State_BlockComment,
        State_BlockComment_Star,

        State_DoubleQuote,
        State_DoubleQuote_Backslash,

        State_LineComment,
        State_LineComment_Backslash,

        State_Normal,

        State_SingleQuote,
        State_SingleQuote_Backslash,

        State_Sharp,
        State_Slash,

        State_Word,

        State_Max_1,
    }; // State

    public: enum Syntax
    {
        Syntax_None,
        Syntax_Comment,
        Syntax_Operator,
        Syntax_String,
        Syntax_Word,
        Syntax_WordReserved,

        Syntax_Max_1,
    }; // Syntax

    private: struct Token
    {
        Syntax  m_eSyntax;
        bool    m_fPartial;
        Posn    m_lEnd;
        Posn    m_lStart;
        Posn    m_lWordStart;
        char16  m_wchPrefix;

        Token()
        {
            Reset();
        } // Token

        void Reset(Posn lPosn = 0)
        {
            m_eSyntax    = Syntax_None;
            m_fPartial   = false;
            m_lEnd       = lPosn;
            m_lStart     = lPosn;
            m_lWordStart = lPosn;
            m_wchPrefix  = 0;
        } // Reset
    }; // Token

    private: static const uint32 k_rgnSyntax2Color[Syntax_Max_1];

    private: State          m_eState;
    private: Token          m_oToken;
    private: char16         m_wchAnnotation;

    // ctor
    protected: ClikeLexer(
        Buffer*         pBuffer,
        KeywordTable*   pKeywordTab,
        const uint*     prgnCharSyntax,
        char16          wchAnnotation ) :
            NewLexer::LexerBase(pBuffer, pKeywordTab, prgnCharSyntax),
            m_eState(State_StartLine),
            m_wchAnnotation(wchAnnotation) {}

    // [C]
    /// <summary>
    ///   Color token.
    /// </summary>
    private: void colorToken()
    {
        setColor(
            m_oToken.m_lStart,
            m_oToken.m_lEnd,
            m_oToken.m_eSyntax,
            k_rgnSyntax2Color[m_oToken.m_eSyntax] );
    } // colorToken

    // [E]
    /// <summary>
    ///   Make token complete.
    /// </summary>
    /// <param name="wch">A character ends token.</param>
    private: Syntax endToken(char16 wch)
    {
        m_oToken.m_lEnd = m_oEnumChar.GetPosn();
        m_eState = 0x0A == wch ? State_StartLine : State_Normal;
        return m_oToken.m_eSyntax;
    } // endToken

    // [G]
    /// <summary>
    ///   Get a token.
    /// </summary>
    private: Syntax getToken()
    {
        if (m_oToken.m_fPartial)
        {
            m_oToken.m_fPartial = false;
        }
        else
        {
            m_oToken.m_eSyntax = Syntax_None;
            m_oToken.m_lStart  = m_oEnumChar.GetPosn();
            m_oToken.m_lEnd    = m_oEnumChar.GetPosn();
        }

        for (;;)
        {
            if (m_oEnumChar.AtLimit())
            {
                // We reached at end of scanning area.
                m_oToken.m_fPartial = true;
                m_oToken.m_lEnd = m_oEnumChar.GetPosn() + 1;
                colorToken();
                return m_oToken.m_eSyntax;
            }

            char16 wch = m_oEnumChar.Get();

          tryAgain:
            switch (m_eState)
            {
            case State_Annotation:
                if (isConsChar(wch))
                {
                    m_oToken.m_eSyntax    = Syntax_Word;
                    m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
                    m_oToken.m_wchPrefix  = m_wchAnnotation;
                    m_eState = State_Word;
                }
                else
                {
                    m_eState = State_Normal;
                    goto tryAgain;
                }
                break;

            case State_BlockComment:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '*':
                    m_eState = State_BlockComment_Star;
                    break;
                } // swtich wch
                break;

            case State_BlockComment_Star:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '*':
                    break;

                case '/':
                    m_oEnumChar.Next();
                    return endToken(wch);

                default:
                    m_eState = State_BlockComment;
                    break;
                } // switch wch
                break;

            case State_DoubleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '"':
                    m_oEnumChar.Next();
                    return endToken(wch);

                case '\\':
                    m_eState = State_DoubleQuote_Backslash;
                    break;
                } // switch wc
                break;

            case State_DoubleQuote_Backslash:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                m_eState = State_DoubleQuote;
                break;

            case State_LineComment:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                switch (wch)
                {
                case 0x0A:
                    // We found end of line comment.
                    return endToken(wch);

                case '\\':
                    m_eState = State_LineComment_Backslash;
                    break;
                } // switch wch
                break;

            case State_LineComment_Backslash:
                ASSERT(Syntax_Comment == m_oToken.m_eSyntax);
                m_eState = State_LineComment;
                break;

            case State_Normal:
                if (0x0A == wch)
                {
                    m_eState = State_StartLine;
                    break;
                }

                if (0x09 == wch || ' ' == wch)
                {
                    break;
                }

                setColor(
                    m_oToken.m_lStart,
                    m_oEnumChar.GetPosn(),
                    Syntax_None,
                    k_rgnSyntax2Color[Syntax_None] );

                m_oToken.m_lStart = m_oEnumChar.GetPosn();

                switch (wch)
                {
                case '"':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_DoubleQuote;
                    break;

                case '\'':
                    m_oToken.m_eSyntax = Syntax_String;
                    m_eState = State_SingleQuote;
                    break;

                case '/':
                    m_oToken.m_eSyntax = Syntax_Operator;
                    m_eState = State_Slash;
                    break;

                default:
                    if (m_wchAnnotation == wch)
                    {
                        m_eState = State_Annotation;
                    }
                    else if (isConsChar(wch))
                    {
                        m_oToken.m_eSyntax = Syntax_Word;
                        m_oToken.m_lWordStart = m_oToken.m_lStart;
                        m_oToken.m_wchPrefix = 0;
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_oEnumChar.Next();
                        m_oToken.m_eSyntax = Syntax_Operator;
                        return endToken(wch);
                    }
                    break;
                } // swtich wch
                break;

            case State_SingleQuote:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                switch (wch)
                {
                case '\'':
                    m_oEnumChar.Next();
                    return endToken(wch);

                case '\\':
                    m_eState = State_SingleQuote_Backslash;
                    break;
                } // switch wc
                break;

            case State_SingleQuote_Backslash:
                ASSERT(Syntax_String == m_oToken.m_eSyntax);
                m_eState = State_SingleQuote;
                break;

            case State_Sharp:
                switch (wch)
                {
                case 0x0A:
                    m_eState = State_StartLine;
                    break;

                case 0x09:
                case 0x20:
                    break;

                default:
                    if (isConsChar(wch))
                    {
                        m_oToken.m_wchPrefix = '#';
                        m_oToken.m_lWordStart = m_oEnumChar.GetPosn();
                        m_eState = State_Word;
                    }
                    else
                    {
                        m_eState = State_Normal;
                    }
                    break;
                } // swtich wch
                break;

            case State_Slash:
                switch (wch)
                {
                case '/':
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_LineComment;
                    break;

                case '*':
                    m_oToken.m_eSyntax = Syntax_Comment;
                    m_eState = State_BlockComment;
                    break;

                default:
                    return endToken(wch);
                } // switch wch
                break;

            case State_StartLine:
                switch (wch)
                {
                case 0x09:
                case 0x0A:
                case 0x20:
                    break;

                case '#':
                    m_oToken.m_eSyntax = Syntax_Word;
                    m_eState = State_Sharp;
                    break;

                default:
                    m_eState = State_Normal;
                    goto tryAgain;
                } // swtich wch
                break;

            case State_Word:
                ASSERT(Syntax_Word == m_oToken.m_eSyntax);
                if (! isConsChar(wch))
                {
                    return endToken(wch);
                } // if
                break;

            default:
                CAN_NOT_HAPPEN();
            } // switch eState

            m_oEnumChar.Next();
        } // for
    } // getToken

    // [P]
    /// <summary>
    ///   Checks token is keyword or not.
    /// </summary>
    private: void processToken()
    {
        if (m_oToken.m_fPartial)
        {
            return;
        }

        switch (m_oToken.m_eSyntax)
        {
        case Syntax_Word:
            break;

        default:
            return;
        } // switch syntax

        char16 wszWord[40];

        int cwchWord = m_oToken.m_lEnd - m_oToken.m_lWordStart + 1;
        if (cwchWord >= lengthof(wszWord))
        {
            return;
        } // if too long word

        char16* pwszWord = wszWord;
        if (0 == m_oToken.m_wchPrefix)
        {
            pwszWord++;
            cwchWord -= 1;
        }
        else
        {
            *pwszWord = m_oToken.m_wchPrefix;
        }

        m_pBuffer->GetText(
            wszWord + 1,
            m_oToken.m_lWordStart,
            m_oToken.m_lEnd );

        StringKey oWord(pwszWord, cwchWord);
        int* piType = m_pKeywordTab->Get(&oWord);
        if (NULL != piType)
        {
            m_oToken.m_eSyntax = Syntax_WordReserved;
        }
    } // processToken

    // [R]
    /// <summary>
    ///   Find restart position.
    /// </summary>
    private: void restart()
    {
        m_oEnumChar.SyncEnd();

        #if DEBUG_LEXER
          DEBUG_PRINTF("Backtrack from %d\n", m_oChange.GetStart());
        #endif

        m_eState   = State_StartLine;

        Posn lStart = 0;
        foreach (
            Buffer::EnumCharRev,
            oEnum,
            Buffer::EnumCharRev::Arg(m_pBuffer, m_oChange.GetStart()) )
        {
            char16 wch = oEnum.Get();
            if (0x20 == wch || 0x09 == wch || 0x0A == wch)
            {
                const StyleValues* pStyle = oEnum.GetStyle();
                if (Syntax_None == pStyle->GetSyntax())
                {
                    lStart = oEnum.GetPosn() - 1;
                    if (0x0A != wch)
                    {
                        m_eState = State_Normal;
                    }
                    break;
                }
            }
        } // for

        #if DEBUG_LEXER
          DEBUG_PRINTF("restart from %d state=%d\n", lStart, m_eState);
        #endif

        m_oToken.Reset(lStart);

        m_oChange.Reset();

        m_oEnumChar.GoTo(lStart);
    } // restart

    /// <summary>
    ///   Lexer entry point.
    /// </summary>
    public: bool Run(Count lCount)
    {
        Posn lChange = m_oChange.GetStart();
        if (m_oEnumChar.GetPosn() >= lChange)
        {
            // The buffer is changed since last scan.
            restart();
        }

        m_oEnumChar.SetCounter(lCount);

        while (! m_oEnumChar.AtLimit())
        {
            getToken();
            processToken();
            colorToken();
        } // while

        return ! m_oEnumChar.AtEnd();
    } // Run
}; // ClikeLexer

//////////////////////////////////////////////////////////////////////
//
// Map Token Type To color
//
const uint32
ClikeLexer::k_rgnSyntax2Color[ClikeLexer::Syntax_Max_1] =
{
    RGB(  0,    0,   0),    // Syntax_None
    RGB(  0,  128,   0),    // Syntax_Comment
    RGB(  0,    0,  33),    // Syntax_Operator
    RGB(163,   21,  21),    // Syntax_String
    RGB(  0,    0,   0),    // Syntax_Word
    RGB(  0,    0, 255),    // Syntax_WordReserved
}; // k_rgnSyntax2Color

/// <summary>
///   C++ lexer
/// </summary>
class CxxLexer : public ClikeLexer
{
    private: static KeywordTable* s_pKeywordTab;

    // ctor
    public: CxxLexer(Buffer* pBuffer) :
        ClikeLexer(
            pBuffer,
            initKeywordTab(),
            k_rgnCxxCharSyntax,
            '#' ) {}

    // [I]
    /// <summary>
    ///   Initialize keyword table.
    /// </summary>
    private: static KeywordTable* initKeywordTab()
    {
        if (NULL == s_pKeywordTab)
        {
            s_pKeywordTab = installKeywords(
                k_rgpwszCKeyword,
                lengthof(k_rgpwszCKeyword) );

            addKeywords(
                s_pKeywordTab,
                k_rgpwszCxxKeyword,
                lengthof(k_rgpwszCxxKeyword) );
        } // if

        return s_pKeywordTab;
    } // CxxLexer
}; //CxxLexer

KeywordTable* CxxLexer::s_pKeywordTab;

/// <summary>
///   C++ mode
/// </summary>
class CxxMode : public Mode
{
    private: CxxLexer m_oLexer;

    // ctor/dtor
    public: CxxMode(ModeFactory* pFactory, Buffer* pBuffer) :
        m_oLexer(pBuffer),
        Mode(pFactory, pBuffer) {}

    // [D]
    public: virtual bool DoColor(Count lCount) override
    {
        return m_oLexer.Run(lCount);
    } // DoColor
}; // CxxMode

/// <summary>
///  Construct CxxModeFactory object
/// </summary>
CxxModeFactory::CxxModeFactory() :
    ModeFactory(k_rgnCxxCharSyntax) {}

/// <summary>
///   Construct CxxMode object.
/// </summary>
Mode* CxxModeFactory::Create(Buffer* pBuffer)
{
    return new CxxMode(this, pBuffer);
} // CxxModeFactory::Create

/// <summary>
///   Java lexer
/// </summary>
class JavaLexer : public ClikeLexer
{
    private: static KeywordTable* s_pKeywordTab;

    // ctor
    public: JavaLexer(Buffer* pBuffer) :
        ClikeLexer(
            pBuffer,
            initKeywordTab(),
            k_rgnCxxCharSyntax,
            '@' ) {}

    // [I]
    /// <summary>
    ///   Initialize keyword table.
    /// </summary>
    private: static KeywordTable* initKeywordTab()
    {
        if (NULL == s_pKeywordTab)
        {
            s_pKeywordTab = installKeywords(
                k_rgpwszJavaKeyword,
                lengthof(k_rgpwszJavaKeyword) );
        } // if

        return s_pKeywordTab;
    } // initKeywordTab
}; //JavaLexer

KeywordTable* JavaLexer::s_pKeywordTab;

/// <summary>
///   Java mode
/// </summary>
class JavaMode : public Mode
{
    private: JavaLexer m_oLexer;

    // ctor/dtor
    public: JavaMode(ModeFactory* pFactory, Buffer* pBuffer) :
        m_oLexer(pBuffer),
        Mode(pFactory, pBuffer) {}

    // [D]
    public: virtual bool DoColor(Count lCount) override
    {
        return m_oLexer.Run(lCount);
    } // DoColor
}; // JavaMode

/// <summary>
///  Construct JavaModeFactory object
/// </summary>
JavaModeFactory::JavaModeFactory() :
    ModeFactory(k_rgnCxxCharSyntax) {}

/// <summary>
///   Construct JavaMode object.
/// </summary>
Mode* JavaModeFactory::Create(Buffer* pBuffer)
{
    return new JavaMode(this, pBuffer);
} // JavaModeFactory::Create

}  // namespace text

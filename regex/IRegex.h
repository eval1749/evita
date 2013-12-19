// -*- Mode: C++ -*-
//
// Regex API
// regex/IRegex.h
//
// Copyright (C) 2007-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/IRegex.h#8 $
//
#if !defined(INCLUDE_IRegex_h)
#define INCLUDE_IRegex_h

namespace Regex
{

typedef wchar_t char16;
typedef long Posn;

#if !defined(interface)
    #define interface struct
#endif

// RegEx_MetaCharacters
// 
//  Description:
//   A string contains mata characters of regular expression.
// 
//      .   match any
//      *   (0, *) quantifier
//      +   (1, *) quantifier
//      ?   (0, 1) quantifier
//      (   start grouping
//      )   end grouping
//      {   (n, m) quanitifer
//       [   character class
//      \   backslash notation
//      ^   match start of string/line
//      $   match end of string/line
//      |   alternation
const char16 MetaCharacters[] = L".+*?(){[\\|^$";

enum Option
{
    Option_None,

    Option_Backward           = 1 << 0,
    Option_ExtendedSyntax     = 1 << 1,     // x
    Option_IgnoreCase         = 1 << 2,     // i
    Option_Multiline          = 1 << 3,     // m
    Option_ExplicitCapture    = 1 << 4,     // n
    Option_Singleline         = 1 << 5,     // s
    Option_Unicode            = 1 << 6,     // u
    Option_ExactString        = 1 << 7,
    Option_ExactWord          = 1 << 8,     // only if ExactString
}; // Option


enum Error
{
    Error_None,

    Error_Eof,                  // 1
    Error_InvalidBackslash,     // 2
    Error_InvalidBrace,         // 3
    Error_InvalidMinMax,        // 4
    Error_InvalidName,          // 5    \k<name> (?<name>...)
    Error_InvalidRange,         // 6
    Error_InvalidQuantifier,    // 7    \b* (zero-width)
    Error_NotSupported,         // 8    (??{code}) (?{code})
    Error_Syntax,               // 9
    Error_UnboundCapture,       // 10    \num or \<name>
    Error_UnclosedPair,         // 11
    Error_NotEnoughMemory,      // 12
}; // Error

struct SourceInfo
{
    Posn    m_lStart;           // for "\A"
    Posn    m_lEnd;             // fot "\Z"
    Posn    m_lScanStart;       // for "^"
    Posn    m_lScanEnd;         // for "$"
}; // SourceInfo

bool /*__fastcall*/ IsAsciiDigitChar(char16);
bool /*__fastcall*/ IsAsciiSpaceChar(char16);
bool /*__fastcall*/ IsAsciiWordChar(char16);

bool /*__fastcall*/ IsUnicodeDigitChar(char16);
bool /*__fastcall*/ IsUnicodeSpaceChar(char16);
bool /*__fastcall*/ IsUnicodeWordChar(char16);

/// <remark>
///  Interface provides chracter tests and overridable implementation.
///  <para>
///     Implementation of methods use Windows API for character case
///     related tests.
///  </para>
/// </remark>
class IEnvironment
{
    public: virtual char16 CharUpcase(char16) const;
    public: virtual char16 CharDowncase(char16) const;
    public: virtual bool   IsBothCase(char16) const;

    public: virtual bool IsAsciiDigitChar(char16 wch) const
        { return Regex::IsAsciiDigitChar(wch); }

    public: virtual bool IsAsciiSpaceChar(char16 wch) const
        { return Regex::IsAsciiSpaceChar(wch); }

    public: virtual bool IsAsciiWordChar(char16 wch) const
        { return Regex::IsAsciiWordChar(wch); }

    public: virtual bool IsUnicodeDigitChar(char16 wch) const
        { return Regex::IsUnicodeDigitChar(wch); }

    public: virtual bool IsUnicodeSpaceChar(char16 wch) const
        { return Regex::IsUnicodeSpaceChar(wch); }

    public: virtual bool IsUnicodeWordChar(char16 wch) const
        { return Regex::IsUnicodeWordChar(wch); }
}; // IEnvironment

/// <remark>
///  Interface of regex compilation context.
/// </remark>
interface ICompileContext : IEnvironment
{
    virtual void* AllocRegex(size_t, int) = 0;
    virtual bool  SetCapture(int, const char16*) = 0;
    virtual void  SetError(int, int) = 0;
}; // ICompileContext

/// <remark>
///  Interface of regex match context.
/// </remark>
interface IMatchContext : IEnvironment
{
    // [B]
    virtual bool BackwardFindCharCi(char16, Posn*, Posn) const = 0;
    virtual bool BackwardFindCharCs(char16, Posn*, Posn) const = 0;

    // [F]
    virtual bool ForwardFindCharCi(char16, Posn*, Posn) const = 0;
    virtual bool ForwardFindCharCs(char16, Posn*, Posn) const = 0;

    // [G]
    virtual bool   GetCapture(int, Posn*, Posn*) const = 0;
    virtual char16 GetChar(Posn) const = 0;
    virtual Posn   GetEnd() const = 0;
    virtual void   GetInfo(SourceInfo*) const = 0;
    virtual Posn   GetStart() const = 0;

    // [R]
    virtual void ResetCapture(int) = 0;
    virtual void ResetCaptures() = 0;

    // [S]
    virtual void SetCapture(int, Posn, Posn) = 0;
    virtual bool StringEqCi(const char16*, int, Posn) const = 0;
    virtual bool StringEqCs(const char16*, int, Posn) const = 0;
}; // IMatchContext

class IRegex;

IRegex* /*__fastcall */ Compile(ICompileContext*, const char16*, int, int = 0);
bool    /* __fastcall */ NextMatch(IRegex*, IMatchContext*);
bool    /* __fastcall */ StartMatch(IRegex*, IMatchContext*);

} // Regex

#endif //!defined(INCLUDE_IRegex_h)

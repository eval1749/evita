#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Regex - Built-In Unicode Support Functions
// regex/regex_unicode.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_unicode.cpp#1 $
//
namespace Regex
{


// isAsciiDigitChar = [0-9]
bool IsAsciiDigitChar(char16 wch)
    { return wch >= '0' && wch <= '9'; }

// IsAsciiSpaceChar = [\x20\f\n\r\t\v]
bool IsAsciiSpaceChar(char16 wch)
    { return 0x20 == wch || (wch >= 0x09 && wch <= 0x0D); }


// IsAsciiWordChar = [a-zA-Z0-9_]
bool IsAsciiWordChar(char16 wch)
{
    return (wch >= 'a' && wch <= 'z') ||
           (wch >= 'A' && wch <= 'Z') ||
           (wch >= '0' && wch <= '9') ||
           '_' == wch;
} // IsAsciiWordChar


// IsUnicodeDigitChar
bool IsUnicodeDigitChar(char16 wch)
{
    uint16 wType;
    if (! ::GetStringTypeW(CT_CTYPE1, &wch, 1, &wType))
    {
        return false;
    }

    return 0 != (wType & C1_DIGIT);
} // IsUnicodeDigitChar

// IsUnicodeSpaceChar
bool IsUnicodeSpaceChar(char16 wch)
{
    uint16 wType;
    if (! ::GetStringTypeW(CT_CTYPE1, &wch, 1, &wType))
    {
        return false;
    }

    return 0 != (wType & (C1_SPACE | C1_BLANK));
} // IsUnicodeSpaceChar

// IsUnicodeWordChar
bool IsUnicodeWordChar(char16 wch)
{
    uint16 wType;
    if (! ::GetStringTypeW(CT_CTYPE1, &wch, 1, &wType))
    {
        return false;
    }

    return 0 != (wType & (C1_ALPHA | C1_DIGIT));
} // IsUnicodeWordChar

} // Regex

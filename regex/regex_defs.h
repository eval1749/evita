//////////////////////////////////////////////////////////////////////////////
//
// Regex - Definitions
// regex_defs.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_defs.h#3 $
//
#ifndef REGEX_REGEX_DEFS_H_
#define REGEX_REGEX_DEFS_H_

#if _DEBUG
#include <stdio.h>
#endif

enum Constants {
  Eof = -1,
  Infinity = 1 << (sizeof(int) * 8 - 2),

  // Named characters
  Newline = 0x0A,

  Backslash = 0x5C,

  CloseBrace = 0x7D,
  CloseBracket = 0x5D,
  CloseParen = 0x29,

  DoubleQuote = 0x22,

  OpenBrace = 0x7B,
  OpenBracket = 0x5B,
  OpenParen = 0x28,

  Quote = 0x27,
};  // Constants

bool IsAsciiDigitChar(char16);
bool IsAsciiSpaceChar(char16);
bool IsAsciiWordChar(char16);

#endif  // REGEX_REGEX_DEFS_H_

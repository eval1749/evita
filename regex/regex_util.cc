//////////////////////////////////////////////////////////////////////////////
//
// Regex - Parser
// regex/regex_util.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_util.cpp#2 $
//
#include "regex/precomp.h"
#include "regex/regex_util.h"

namespace Regex {
namespace RegexPrivate {

char16* lstrchrW(const char16* pwsz, char16 wch) {
  while (0 != *pwsz) {
    if (*pwsz == wch) {
      return const_cast<char16*>(pwsz);
    }
    pwsz++;
  }
  return NULL;
}

bool IsWhitespace(char16 wch) {
  return NULL != lstrchrW(L" \x09\x0A\xC\x0D", wch);
}

}  // namespace RegexPrivate
}  // namespace Regex

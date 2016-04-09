// Copyright 1996-2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/regex/regex_util.h"

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

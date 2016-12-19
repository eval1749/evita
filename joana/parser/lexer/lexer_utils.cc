// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/lexer/lexer_utils.h"

namespace joana {
namespace internal {

int FromDigitChar(base::char16 char_code, int base) {
  DCHECK_GE(base, 2);
  DCHECK_LE(base, 16);
  if (base == 16) {
    if (char_code >= '0' && char_code <= '9')
      return char_code - '0';
    if (char_code >= 'A' && char_code <= 'F')
      return char_code - 'A' + 10;
    if (char_code >= 'a' && char_code <= 'f')
      return char_code - 'a' + 10;
    NOTREACHED() << char_code;
    return 0;
  }
  if (char_code >= '0' && char_code <= '0' + base - 1)
    return char_code - '0';
  NOTREACHED() << char_code;
  return 0;
}

bool IsDigitChar(base::char16 char_code, int base) {
  DCHECK_GE(base, 2);
  DCHECK_LE(base, 16);
  if (base == 16) {
    if (char_code >= '0' && char_code <= '9')
      return true;
    if (char_code >= 'A' && char_code <= 'F')
      return true;
    return char_code >= 'a' && char_code <= 'f';
  }
  return char_code >= '0' && char_code <= '0' + base - 1;
}

bool IsIdentifierPart(base::char16 char_code) {
  if (char_code >= '0' && char_code <= '9')
    return true;
  if (char_code >= 'A' && char_code <= 'Z')
    return true;
  if (char_code >= 'a' && char_code <= 'z')
    return true;
  // TODO(eval1749): NYI: UnicodeIDContinue
  // TODO(eval1749): NYI: \ UnicodeEscapeSequence
  // TODO(eval1749): NYI: <ZWNJ> <ZWJ>
  return char_code == '$' || char_code == '_';
}

bool IsLineTerminator(base::char16 char_code) {
  return char_code == '\n' || char_code == '\r' ||
         char_code == kLineSeparator || char_code == kParagraphSeparator;
}

bool IsWhitespace(base::char16 char_code) {
  return IsLineTerminator(char_code) || char_code == ' ' || char_code == '\t' ||
         char_code == '\v' || char_code == '\f' || char_code == '\r';
}

}  // namespace internal
}  // namespace joana

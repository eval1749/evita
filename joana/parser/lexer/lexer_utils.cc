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

}  // namespace internal
}  // namespace joana

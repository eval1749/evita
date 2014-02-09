// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_char_syntax_h)
#define INCLUDE_evita_dom_modes_char_syntax_h

#include "base/basictypes.h"

namespace text {

namespace CharSyntax {

enum Syntax {
  Syntax_None = 0x0,

  Syntax_CloseParen = 0x1,
  Syntax_Control = 0x2,
  Syntax_Escape = 0x3,
  Syntax_LineComment = 0x4,
  Syntax_OpenParen = 0x5,
  Syntax_Punctuation = 0x6,
  Syntax_StringQuote = 0x7,
  Syntax_Whitespace = 0x8,
  Syntax_Word = 0x9,
};

enum Constant {
  Trait_PairShift = 4,
  Trait_SyntaxMask = (1 << Trait_PairShift) - 1,

  Trait_BlockCommentEnd1 = 0x0001 << 16,
  Trait_BlockCommentEnd2 = 0x0002 << 16,
  Trait_BlockCommentEnd = 0x0003 << 16,
  Trait_BlockCommentStart1 = 0x0010 << 16,
  Trait_BlockCommentStart2 = 0x0020 << 16,
  Trait_BlockCommentStart = 0x0030 << 16,
  Trait_LineCommentStart = 0x0100 << 16,
};

inline base::char16 GetPairChar(uint32_t nSyntax) {
  return static_cast<char16>((nSyntax >> Trait_PairShift) & 0xFF);
}

inline Syntax GetSyntax(uint32_t nSyntax) {
  return static_cast<Syntax>(nSyntax & Trait_SyntaxMask);
}
}  // namespace CharSyntax
}  // namespace text

#endif //!defined(INCLUDE_evita_dom_modes_char_syntax_h)

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_LEXER_UTILS_H_
#define JOANA_PARSER_LEXER_LEXER_UTILS_H_

#include "base/logging.h"
#include "base/strings/string16.h"

namespace joana {
namespace internal {

constexpr auto kBackslash = '\\';
constexpr auto kLeftBrace = '{';
constexpr auto kRightBrace = '}';
constexpr auto kLeftBracket = '[';
constexpr auto kRightBracket = ']';
constexpr auto kLeftParenthesis = '(';
constexpr auto kRightParenthesis = ')';
constexpr auto kLineSeparator = 0x2028;
constexpr auto kParagraphSeparator = 0x2029;
constexpr auto kMaxUnicodeCodePoint = 0x10FFFF;

// Returns a digit from digit character |char_code| in |base|. It is error
// if |char_code| is not valid digit character.
int FromDigitChar(base::char16 char_code, int base);

// Returns true if |char_code| reprsents a digit of |base|. |base| >= 2 &&
// |base| <= 16.
bool IsDigitChar(base::char16 char_code, int base);

// Returns true if |char_code| is valid for ECMAScript identifier part.
bool IsIdentifierPart(base::char16 char_code);

// Returns true if |char_code| is valid for ECMAScript identifier start.
bool IsIdentifierStart(base::char16 char_code);

// Returns true if |char_code| is a line terminator of ECMAScript.
bool IsLineTerminator(base::char16 char_code);

// Returns true if |char_code| is a whitespace of ECMAScript.
bool IsWhitespace(base::char16 char_code);

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_LEXER_UTILS_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_ERROR_CODES_H_
#define JOANA_PUBLIC_AST_ERROR_CODES_H_

#include "base/strings/string_piece.h"
#include "joana/public/public_export.h"

namespace joana {
namespace ast {

// V(token, reason)
#define FOR_EACH_LEXER_ERROR_CODE(V)     \
  V(BLOCK_COMMENT, NOT_CLOSED)           \
  V(CHARACTER, INVALID)                  \
  V(CHARACTER, NO_MORE)                  \
  V(NUMERIC_LITERAL, INTEGER_BAD_DIGIT)  \
  V(NUMERIC_LITERAL, INTEGER_NO_DIGITS)  \
  V(NUMERIC_LITERAL, INTEGER_OCTAL)      \
  V(NUMERIC_LITERAL, INTEGER_OVERFLOW)   \
  V(NUMERIC_LITERAL, PREFIX_ZERO)        \
  V(PUNCTUATOR, DOT_DOT)                 \
  V(STRING_LITERAL, BACKSLASH)           \
  V(STRING_LITERAL, BACKSLASH_HEX_DIGIT) \
  V(STRING_LITERAL, BACKSLASH_UNICODE)   \
  V(STRING_LITERAL, NEWLINE)             \
  V(STRING_LITERAL, NOT_CLOSED)

#define FOR_EACH_PARSER_ERROR_CODE(V)   \
  V(BRACKET, MISMATCHED)                \
  V(BRACKET, NOT_CLOSED)                \
  V(BRACKET, UNEXPECTED)                \
  V(EXPRESSION, INVALID)                \
  V(EXPRESSION, PARENTHESIS_NOT_CLSOED) \
  V(STATEMENT, AWAIT)                   \
  V(STATEMENT, BRACE_NOT_CLSOED)        \
  V(STATEMENT, BREAK_BAD_PLACE)         \
  V(STATEMENT, BREAK_NOT_LABEL)         \
  V(STATEMENT, BREAK_SEMI_COLON)        \
  V(STATEMENT, CASE)                    \
  V(STATEMENT, CATCH)                   \
  V(STATEMENT, CONTINUE_BAD_PLACE)      \
  V(STATEMENT, CONTINUE_NOT_LABEL)      \
  V(STATEMENT, CONTINUE_SEMI_COLON)     \
  V(STATEMENT, DO_EXPECT_LPAREN)        \
  V(STATEMENT, DO_EXPECT_RPAREN)        \
  V(STATEMENT, DO_EXPECT_SEMI_COLON)    \
  V(STATEMENT, DO_EXPECT_WHILE)         \
  V(STATEMENT, ELSE)                    \
  V(STATEMENT, EXPECT_SEMI_COLON)       \
  V(STATEMENT, EXTENDS)                 \
  V(STATEMENT, FINALLY)                 \
  V(STATEMENT, IF_EXPECT_LPAREN)        \
  V(STATEMENT, IF_EXPECT_RPAREN)        \
  V(STATEMENT, INVALID)                 \
  V(STATEMENT, RESERVED_WORD)           \
  V(STATEMENT, YIELD)

const auto kLexerErrorCodeBase = 10000;
const auto kParserErrorCodeBase = 20000;

JOANA_PUBLIC_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_ERROR_CODES_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_ERROR_CODES_H_
#define JOANA_PUBLIC_AST_ERROR_CODES_H_

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
  V(EXPRESSION, BRACKET_NOT_CLSOED)     \
  V(EXPRESSION, PARENTHESIS_NOT_CLSOED) \
  V(EXPRESSION, NYI)                    \
  V(STATEMENT, BRACE_NOT_CLSOED)        \
  V(STATEMENT, NO_SEMI_COLON)           \
  V(STATEMENT, NYI)

const auto kLexerErrorCodeBase = 10000;
const auto kParserErrorCodeBase = 20000;

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_ERROR_CODES_H_

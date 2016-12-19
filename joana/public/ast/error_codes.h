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
  V(NUMERIC_LITERAL, DECIMAL_BAD_DIGIT)  \
  V(NUMERIC_LITERAL, INTEGER_BAD_DIGIT)  \
  V(NUMERIC_LITERAL, INTEGER_NO_DIGITS)  \
  V(NUMERIC_LITERAL, INTEGER_OCTAL)      \
  V(NUMERIC_LITERAL, INTEGER_OVERFLOW)   \
  V(NUMERIC_LITERAL, PREFIX_ZERO)        \
  V(PUNCTUATOR, DOT_DOT)                 \
  V(REGEXP, EXPECT_CHAR)                 \
  V(REGEXP, EXPECT_PATTERN)              \
  V(REGEXP, EXPECT_PRIMARY)              \
  V(REGEXP, EXPECT_RBRACE)               \
  V(REGEXP, EXPECT_RBRACKET)             \
  V(REGEXP, EXPECT_RPAREN)               \
  V(REGEXP, INVALID_GROUPING)            \
  V(REGEXP, INVALID_NUMBER)              \
  V(REGEXP, INVALID_OR)                  \
  V(REGEXP, INVALID_REPEAT)              \
  V(REGEXP, INVALID_SEQUENCE)            \
  V(REGEXP, UNEXPECT_RPAREN)             \
  V(STRING_LITERAL, BACKSLASH)           \
  V(STRING_LITERAL, BACKSLASH_HEX_DIGIT) \
  V(STRING_LITERAL, BACKSLASH_UNICODE)   \
  V(STRING_LITERAL, NEWLINE)             \
  V(STRING_LITERAL, NOT_CLOSED)

#define FOR_EACH_PARSER_ERROR_CODE(V)        \
  V(BRACKET, MISMATCHED)                     \
  V(BRACKET, NOT_CLOSED)                     \
  V(BRACKET, UNEXPECTED)                     \
  V(CLASS, EXPECT_LBRACE)                    \
  V(CLASS, INVALID)                          \
  V(EXPRESSION, ARGUMENT_LIST_EXPECT_COMMA)  \
  V(EXPRESSION, ARGUMENT_LIST_EXPECT_RPAREN) \
  V(EXPRESSION, CONDITIONAL_EXPECT_COLON)    \
  V(EXPRESSION, EXPECT_EXPRESSION)           \
  V(EXPRESSION, EXPECT_NAME)                 \
  V(EXPRESSION, EXPECT_RBRACKET)             \
  V(EXPRESSION, INVALID)                     \
  V(EXPRESSION, PARENTHESIS_NOT_CLSOED)      \
  V(EXPRESSION, PRIMARY_EXPECT_ARROW)        \
  V(EXPRESSION, PRIMARY_EXPECT_RPAREN)       \
  V(EXPRESSION, UPDATE_EXPECT_LHS)           \
  V(FUNCTION, EXPECT_LBRACE)                 \
  V(FUNCTION, EXPECT_LPAREN)                 \
  V(FUNCTION, EXPECT_RPAREN)                 \
  V(FUNCTION, INVALID_ARROW_FUNCTION_BODY)   \
  V(FUNCTION, INVALID_PARAMETER)             \
  V(FUNCTION, INVALID_PARAMETER_LIST)        \
  V(PROPERTY, EXPECT_COMMA)                  \
  V(PROPERTY, EXPECT_RBRACKET)               \
  V(PROPERTY, INVALID_PROPERTY_NAME)         \
  V(PROPERTY, INVALID_TOKEN)                 \
  V(PROPERTY, INVALID_STATIC)                \
  V(STATEMENT, CATCH_EXPECT_NAME)            \
  V(STATEMENT, EXPECT_CATCH)                 \
  V(STATEMENT, EXPECT_COLON)                 \
  V(STATEMENT, EXPECT_LABEL)                 \
  V(STATEMENT, EXPECT_LBRACE)                \
  V(STATEMENT, EXPECT_LPAREN)                \
  V(STATEMENT, EXPECT_RBRACE)                \
  V(STATEMENT, EXPECT_RPAREN)                \
  V(STATEMENT, EXPECT_SEMICOLON)             \
  V(STATEMENT, EXPECT_STATEMENT)             \
  V(STATEMENT, EXPECT_WHILE)                 \
  V(STATEMENT, INVALID)                      \
  V(STATEMENT, RESERVED_WORD)                \
  V(STATEMENT, UNEXPECT_NEWLINE)             \
  V(STATEMENT, SWITCH_INVALID_CLAUSE)

const auto kLexerErrorCodeBase = 10000;
const auto kParserErrorCodeBase = 20000;

JOANA_PUBLIC_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_ERROR_CODES_H_

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

#define FOR_EACH_PARSER_ERROR_CODE(V)        \
  V(BRACKET, MISMATCHED)                     \
  V(BRACKET, NOT_CLOSED)                     \
  V(BRACKET, UNEXPECTED)                     \
  V(EXPRESSION, ARGUMENT_LIST_EXPECT_COMMA)  \
  V(EXPRESSION, ARGUMENT_LIST_EXPECT_RPAREN) \
  V(EXPRESSION, ASSIGNMENT_EXPECT_LHS)       \
  V(EXPRESSION, ASYNC_EXPECT_FUNCTION)       \
  V(EXPRESSION, CONDITIONAL_EXPECT_COLON)    \
  V(EXPRESSION, LHS_EXPECT_NAME)             \
  V(EXPRESSION, INVALID)                     \
  V(EXPRESSION, LHS_EXPECT_RBRACKET)         \
  V(EXPRESSION, NEW_EXPECT_TARGET)           \
  V(EXPRESSION, PARENTHESIS_NOT_CLSOED)      \
  V(EXPRESSION, PRIMARY_EXPECT_ARROW)        \
  V(EXPRESSION, PRIMARY_EXPECT_RPAREN)       \
  V(EXPRESSION, UPDATE_EXPECT_LHS)           \
  V(FUNCTION, EXPECT_FUNCTION)               \
  V(FUNCTION, EXPECT_LBRACE)                 \
  V(FUNCTION, EXPECT_LPAREN)                 \
  V(FUNCTION, EXPECT_RPAREN)                 \
  V(FUNCTION, INVALID_ARROW_FUNCTION_BODY)   \
  V(FUNCTION, INVALID_PARAMETER)             \
  V(FUNCTION, INVALID_PARAMETER_LIST)        \
  V(STATEMENT, AWAIT)                        \
  V(STATEMENT, BRACE_NOT_CLSOED)             \
  V(STATEMENT, BREAK_NOT_LABEL)              \
  V(STATEMENT, BREAK_SEMI_COLON)             \
  V(STATEMENT, CASE_EXPECT_COLON)            \
  V(STATEMENT, CATCH)                        \
  V(STATEMENT, CATCH_EXPECT_LBRACE)          \
  V(STATEMENT, CATCH_EXPECT_LPAREN)          \
  V(STATEMENT, CATCH_EXPECT_NAME)            \
  V(STATEMENT, CATCH_EXPECT_RPAREN)          \
  V(STATEMENT, CONTINUE_NOT_LABEL)           \
  V(STATEMENT, CONTINUE_SEMI_COLON)          \
  V(STATEMENT, DEFAULT_EXPECT_COLON)         \
  V(STATEMENT, DO_EXPECT_LPAREN)             \
  V(STATEMENT, DO_EXPECT_RPAREN)             \
  V(STATEMENT, DO_EXPECT_SEMI_COLON)         \
  V(STATEMENT, DO_EXPECT_WHILE)              \
  V(STATEMENT, ELSE)                         \
  V(STATEMENT, EXPECT_SEMI_COLON)            \
  V(STATEMENT, EXTENDS)                      \
  V(STATEMENT, FINALLY)                      \
  V(STATEMENT, FINALLY_EXPECT_LBRACE)        \
  V(STATEMENT, FINALLY_EXPECT_LPAREN)        \
  V(STATEMENT, IF_EXPECT_LPAREN)             \
  V(STATEMENT, IF_EXPECT_RPAREN)             \
  V(STATEMENT, INVALID)                      \
  V(STATEMENT, RESERVED_WORD)                \
  V(STATEMENT, SWITCH_EXPECT_LBRACE)         \
  V(STATEMENT, SWITCH_EXPECT_LPAREN)         \
  V(STATEMENT, SWITCH_EXPECT_RBRACE)         \
  V(STATEMENT, SWITCH_EXPECT_RPAREN)         \
  V(STATEMENT, SWITCH_INVALID_CLAUSE)        \
  V(STATEMENT, THROW_EXPECT_SEMI_COLON)      \
  V(STATEMENT, TRY_EXPECT_CATCH)             \
  V(STATEMENT, TRY_EXPECT_LBRACE)            \
  V(STATEMENT, TRY_EXPECT_LPAREN)            \
  V(STATEMENT, YIELD)

const auto kLexerErrorCodeBase = 10000;
const auto kParserErrorCodeBase = 20000;

JOANA_PUBLIC_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_ERROR_CODES_H_

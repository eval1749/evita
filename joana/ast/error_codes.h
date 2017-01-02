// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_ERROR_CODES_H_
#define JOANA_AST_ERROR_CODES_H_

#include "base/strings/string_piece.h"
#include "joana/ast/ast_export.h"

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
  V(REGEXP, EXPECT_SLASH)                \
  V(REGEXP, INVALID_GROUPING)            \
  V(REGEXP, INVALID_NUMBER)              \
  V(REGEXP, INVALID_OR)                  \
  V(REGEXP, INVALID_REPEAT)              \
  V(STRING_LITERAL, BACKSLASH)           \
  V(STRING_LITERAL, BACKSLASH_HEX_DIGIT) \
  V(STRING_LITERAL, BACKSLASH_UNICODE)   \
  V(STRING_LITERAL, NEWLINE)             \
  V(STRING_LITERAL, NOT_CLOSED)

#define FOR_EACH_JSDOC_ERROR_CODE(V) \
  V(JSDOC, EXPECT_LBRACE)            \
  V(JSDOC, EXPECT_RBRACE)            \
  V(JSDOC, UNKNOWN_TAG)

#define FOR_EACH_PARSER_ERROR_CODE(V)        \
  V(BINDING, EXPECT_COMMA)                   \
  V(BINDING, INVALID_ELEMENT)                \
  V(BINDING, UNEXPECT_COMMA)                 \
  V(BINDING, UNEXPECT_REST)                  \
  V(BRACKET, EXPECT_RBRACE)                  \
  V(BRACKET, EXPECT_RBRACKET)                \
  V(BRACKET, EXPECT_RPAREN)                  \
  V(BRACKET, UNEXPECT_RBRACE)                \
  V(BRACKET, UNEXPECT_RBRACKET)              \
  V(BRACKET, UNEXPECT_RPAREN)                \
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
  V(EXPRESSION, UNEXPECT_ANNOTATION)         \
  V(EXPRESSION, UNEXPECT_NEWLINE)            \
  V(EXPRESSION, UPDATE_EXPECT_LHS)           \
  V(FUNCTION, EXPECT_LBRACE)                 \
  V(FUNCTION, EXPECT_LPAREN)                 \
  V(FUNCTION, EXPECT_RPAREN)                 \
  V(FUNCTION, INVALID_ARROW_FUNCTION_BODY)   \
  V(FUNCTION, INVALID_PARAMETER)             \
  V(FUNCTION, INVALID_PARAMETER_LIST)        \
  V(JSDOC, MULTIPLE_FILE_OVERVIEWS)          \
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
  V(STATEMENT, UNEXPECT_ANNOTATION)          \
  V(STATEMENT, UNEXPECT_NEWLINE)             \
  V(STATEMENT, SWITCH_INVALID_CLAUSE)

#define FOR_EACH_TYPE_ERROR_CODE(V) \
  V(TYPE, EXPECT_COLON)             \
  V(TYPE, EXPECT_COMMA)             \
  V(TYPE, EXPECT_LBRACE)            \
  V(TYPE, EXPECT_RANGLE)            \
  V(TYPE, EXPECT_RBRACE)            \
  V(TYPE, EXPECT_RBRACKET)          \
  V(TYPE, EXPECT_RPAREN)            \
  V(TYPE, EXPECT_TYPE)              \
  V(TYPE, UNEXPECT_DOT)             \
  V(TYPE, UNEXPECT_RANGLE)          \
  V(TYPE, UNEXPECT_RBRACE)          \
  V(TYPE, UNEXPECT_RBRACKET)        \
  V(TYPE, UNEXPECT_RPAREN)

const auto kLexerErrorCodeBase = 10000;
const auto kParserErrorCodeBase = 20000;
const auto kJsDocErrorCodeBase = 30000;
const auto kTypeErrorCodeBase = 40000;

JOANA_AST_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_ERROR_CODES_H_

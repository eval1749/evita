// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>
#include <vector>

#include "joana/parser/lexer/lexer.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/literals.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"
#include "joana/parser/jsdoc/jsdoc_parser.h"
#include "joana/parser/public/parse.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/simple_formatter.h"

namespace joana {
namespace parser {

namespace {

enum ErrorCode {
  NONE = ast::kLexerErrorCodeBase,
#define V(token, reason) ERROR_##token##_##reason,
  FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
};

std::string ToString(const ast::Node& node) {
  std::ostringstream ostream;
  ostream << node;
  return ostream.str();
}

}  // namespace

//
// LexerTest
//
class LexerTest : public LexerTestBase {
 protected:
  enum class LexerMode {
    Normal,
    RegExp,
  };

  LexerTest() = default;
  ~LexerTest() override = default;

  SourceCodeRange MakeRange(int start, int end) const;
  SourceCodeRange MakeRange() const;
  std::string NewComment(int start, int end);
  std::string NewComment();
  std::string NewError(ErrorCode error_code, int start, int end);
  std::string NewName(int start, int end);
  std::string NewNumericLiteral(int start, int end, double value);
  std::string NewNumericLiteral(double value);
  std::string NewPunctuator(ast::TokenKind kind);
  std::string NewRegExpSource(int start, int end);
  std::string NewRegExpSource();
  std::string NewStringLiteral(int start, int end, base::StringPiece data);
  std::string NewStringLiteral(base::StringPiece data);
  std::string NewStringLiteral(const std::vector<base::char16> data);
  std::string Parse(const ParserOptions& options,
                    LexerMode mode = LexerMode::Normal);
  std::string Parse(LexerMode mode = LexerMode::Normal);

 private:
  DISALLOW_COPY_AND_ASSIGN(LexerTest);
};

SourceCodeRange LexerTest::MakeRange(int start, int end) const {
  return source_code().Slice(start, end);
}

SourceCodeRange LexerTest::MakeRange() const {
  return source_code().range();
}

std::string LexerTest::NewComment(int start, int end) {
  return ToString(node_factory().NewComment(MakeRange(start, end)));
}

std::string LexerTest::NewComment() {
  return ToString(node_factory().NewComment(MakeRange()));
}

std::string LexerTest::NewError(ErrorCode error_code, int start, int end) {
  std::ostringstream ostream;
  ostream << ' ' << error_code << '@' << source_code().Slice(start, end);
  return ostream.str();
}

std::string LexerTest::NewName(int start, int end) {
  return ToString(node_factory().NewName(source_code().Slice(start, end)));
}

std::string LexerTest::NewNumericLiteral(int start, int end, double value) {
  return ToString(
      node_factory().NewNumericLiteral(MakeRange(start, end), value));
}

std::string LexerTest::NewNumericLiteral(double value) {
  return ToString(node_factory().NewNumericLiteral(MakeRange(), value));
}

std::string LexerTest::NewPunctuator(ast::TokenKind kind) {
  return ToString(node_factory().NewPunctuator(MakeRange(), kind));
}

std::string LexerTest::NewRegExpSource(int start, int end) {
  return ToString(node_factory().NewRegExpSource(MakeRange(start, end)));
}

std::string LexerTest::NewRegExpSource() {
  return ToString(node_factory().NewRegExpSource(MakeRange()));
}

std::string LexerTest::NewStringLiteral(int start,
                                        int end,
                                        base::StringPiece data8) {
  return ToString(node_factory().NewStringLiteral(MakeRange(start, end)));
}

std::string LexerTest::NewStringLiteral(base::StringPiece data8) {
  return ToString(node_factory().NewStringLiteral(MakeRange()));
}

std::string LexerTest::NewStringLiteral(const std::vector<base::char16> data) {
  return ToString(node_factory().NewStringLiteral(MakeRange()));
}

std::string LexerTest::Parse(const ParserOptions& options, LexerMode mode) {
  Lexer lexer(&context(), source_code().range(), options);
  std::ostringstream ostream;
  auto delimiter = "";
  while (lexer.CanPeekToken()) {
    ostream << delimiter;
    delimiter = " ";
    const auto& node = lexer.PeekToken();
    if (node == ast::TokenKind::Divide || node == ast::TokenKind::DivideEqual) {
      if (mode == LexerMode::RegExp)
        lexer.ExtendTokenAsRegExp();
    }
    ostream << AsFormatted(lexer.ConsumeToken());
  }
  for (const auto* error : error_sink().errors()) {
    ostream << ' ' << error->error_code() << '@' << error->range();
  }
  return ostream.str();
}

std::string LexerTest::Parse(LexerMode mode) {
  return Parse({}, mode);
}

TEST_F(LexerTest, JsDoc) {
  PrepareSouceCode("/** @type {number} */");
  const auto& document =
      *JsDocParser(&context(), source_code().range(), ParserOptions()).Parse();
  EXPECT_EQ(ToString(document), Parse());

  PrepareSouceCode("/** */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/*** */");
  EXPECT_EQ(NewComment(), Parse()) << "Starts with thee '*' is not JsDoc.";
}

TEST_F(LexerTest, BlockComment) {
  PrepareSouceCode("/**/");
  EXPECT_EQ(NewComment(), Parse()) << "/**/ is a block comment";

  PrepareSouceCode("/* abc */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* * */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* ** */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* ***/");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* ");
  EXPECT_EQ(NewComment(0, 3) + NewError(ERROR_BLOCK_COMMENT_NOT_CLOSED, 0, 3),
            Parse())
      << "Open block comment";
}

TEST_F(LexerTest, Empty) {
  PrepareSouceCode("");
  EXPECT_EQ("", Parse());
}

TEST_F(LexerTest, InvalidChar) {
  PrepareSouceCode("#foo");
  EXPECT_EQ(NewName(1, 4) + NewError(ERROR_CHARACTER_INVALID, 0, 1), Parse())
      << "Skip invalid character and return token";

  PrepareSouceCode("#");
  EXPECT_EQ(NewError(ERROR_CHARACTER_INVALID, 0, 1), Parse())
      << "Invalid character at end of source";
}

TEST_F(LexerTest, LineComment) {
  PrepareSouceCode("//");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("//\n");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("//abc\n");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("//abc\n123");
  EXPECT_EQ(NewComment(0, 6) + " " + NewNumericLiteral(6, 9, 123), Parse());
}

TEST_F(LexerTest, Name) {
  PrepareSouceCode("abc");
  EXPECT_EQ(NewName(0, 3), Parse());

  PrepareSouceCode("abc def");
  EXPECT_EQ(NewName(0, 3) + " " + NewName(4, 7), Parse());

  PrepareSouceCode("while");
  EXPECT_EQ(NewName(0, 5), Parse());
}

TEST_F(LexerTest, NodeFactoryNewName) {
  PrepareSouceCode("while");
  EXPECT_EQ(static_cast<int>(ast::TokenKind::While),
            node_factory().NewName(MakeRange(0, 5)).name_id())
      << "We can identify keyword 'while'.";

  PrepareSouceCode("while");
  EXPECT_TRUE(
      ast::NameSyntax::IsKeyword(node_factory().NewName(MakeRange(0, 5))));

  PrepareSouceCode("from");
  EXPECT_EQ(static_cast<int>(ast::TokenKind::From),
            node_factory().NewName(MakeRange(0, 4)).name_id())
      << "We can identify contextual keyword 'from'.";

  PrepareSouceCode("from");
  EXPECT_FALSE(
      ast::NameSyntax::IsKeyword(node_factory().NewName(MakeRange(0, 4))));

  PrepareSouceCode("of");
  EXPECT_EQ(static_cast<int>(ast::TokenKind::Of),
            node_factory().NewName(MakeRange(0, 2)).name_id())
      << "We can identify contextual keyword 'of'.";
}

TEST_F(LexerTest, NumericLiteral) {
  PrepareSouceCode("1234");
  EXPECT_EQ(NewNumericLiteral(1234), Parse());

  PrepareSouceCode("0 1");
  EXPECT_EQ(NewNumericLiteral(0, 1, 0) + " " + NewNumericLiteral(2, 3, 1),
            Parse());

  PrepareSouceCode("1 2");
  EXPECT_EQ(NewNumericLiteral(0, 1, 1) + " " + NewNumericLiteral(2, 3, 2),
            Parse());

  PrepareSouceCode("0b1011");
  EXPECT_EQ(NewNumericLiteral(0b1011), Parse());

  PrepareSouceCode("0o1234");
  EXPECT_EQ(NewNumericLiteral(01234), Parse());

  PrepareSouceCode("0x1234");
  EXPECT_EQ(NewNumericLiteral(0x1234), Parse());

  PrepareSouceCode("12345678901234567890");
  EXPECT_EQ(NewNumericLiteral(1.23457e+19), Parse());

  PrepareSouceCode("0.5");
  EXPECT_EQ(NewNumericLiteral(0.5), Parse());

  PrepareSouceCode("0e3");
  EXPECT_EQ(NewNumericLiteral(0), Parse());

  PrepareSouceCode(".123");
  EXPECT_EQ(NewNumericLiteral(.123), Parse());

  // From protobuf/js/binary/proto_test.js
  PrepareSouceCode("0X7FFFFFFF00000000");
  EXPECT_EQ(NewNumericLiteral(0X7FFFFFFF00000000), Parse())
      << "More 0's does not cause overflow";
}

TEST_F(LexerTest, NumericLiteralError) {
  PrepareSouceCode("0b1034");
  EXPECT_EQ(NewNumericLiteral(0, 6, 2) +
                NewError(ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT, 4, 6),
            Parse())
      << "Binary literal contains 3 and 4";

  PrepareSouceCode("0o128934");
  EXPECT_EQ(NewNumericLiteral(0, 8, 10) +
                NewError(ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT, 4, 8),
            Parse())
      << "Octal literal contains 8 and 9";

  PrepareSouceCode("0x12X34");
  EXPECT_EQ(NewNumericLiteral(0, 7, 18) +
                NewError(ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT, 4, 7),
            Parse())
      << "Hexadecimal literal contains 'X'";

  PrepareSouceCode("0x");
  EXPECT_EQ(NewNumericLiteral(0, 2, 0) +
                NewError(ERROR_NUMERIC_LITERAL_INTEGER_NO_DIGITS, 0, 2),
            Parse())
      << "No digits after '0x'";

  PrepareSouceCode("0123");
  EXPECT_EQ(NewNumericLiteral(0, 4, 83) +
                NewError(ERROR_NUMERIC_LITERAL_INTEGER_OCTAL, 0, 4),
            Parse())
      << "Strict mode does not allow legacy octal literal";

  PrepareSouceCode("0f0");
  EXPECT_EQ(NewNumericLiteral(0, 3, 0) +
                NewError(ERROR_NUMERIC_LITERAL_DECIMAL_BAD_DIGIT, 1, 3),
            Parse());

  PrepareSouceCode("12x43");
  EXPECT_EQ(NewNumericLiteral(0, 5, 12) +
                NewError(ERROR_NUMERIC_LITERAL_DECIMAL_BAD_DIGIT, 2, 5),
            Parse())
      << "Decimal contains letter.";
}

TEST_F(LexerTest, Punctuator) {
  // ";"
  PrepareSouceCode(";");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // ":"
  PrepareSouceCode(":");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Colon), Parse());

  // "=>"
  PrepareSouceCode("=>");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Arrow), Parse());

  // "("
  PrepareSouceCode("(");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // ")"
  PrepareSouceCode(")");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // "?"
  PrepareSouceCode("?");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Question), Parse());

  // "~"
  PrepareSouceCode("~");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::BitNot), Parse());

  // "["
  PrepareSouceCode("[");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // "]"
  PrepareSouceCode("]");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // "{"
  PrepareSouceCode("{");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // "}"
  PrepareSouceCode("}");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Semicolon), Parse());

  // "."
  PrepareSouceCode(".");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Dot), Parse());

  PrepareSouceCode("...");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::DotDotDot), Parse());

  // "!"
  PrepareSouceCode("!");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::LogicalNot), Parse());

  PrepareSouceCode("!=");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::NotEqual), Parse());

  PrepareSouceCode("!==");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::NotEqualEqual), Parse());

  // "+"
  PrepareSouceCode("+");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::Plus), Parse());

  PrepareSouceCode("++");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::PlusPlus), Parse());

  PrepareSouceCode("+=");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::PlusPlus), Parse());

  // "<"
  PrepareSouceCode("<");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::LessThan), Parse());

  PrepareSouceCode("<<");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::LeftShift), Parse());

  PrepareSouceCode("<=");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::LessThanOrEqual), Parse());

  // ">"
  PrepareSouceCode(">");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::GreaterThan), Parse());

  PrepareSouceCode(">>");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::RightShift), Parse());

  PrepareSouceCode(">>>");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::UnsignedRightShift), Parse());

  PrepareSouceCode(">=");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::GreaterThanOrEqual), Parse());
}

TEST_F(LexerTest, PunctuatorError) {
  PrepareSouceCode("..");
  EXPECT_EQ(NewPunctuator(ast::TokenKind::DotDot) +
                NewError(ERROR_PUNCTUATOR_DOT_DOT, 0, 2),
            Parse())
      << "'..' is not a valid punctuator.";
}

TEST_F(LexerTest, RegExp) {
  PrepareSouceCode("/=/");
  EXPECT_EQ(NewRegExpSource(), Parse(LexerMode::RegExp))
      << "regexp starts with '='";

  PrepareSouceCode("/foo/");
  EXPECT_EQ(NewRegExpSource(), Parse(LexerMode::RegExp));

  PrepareSouceCode("/[/]/");
  EXPECT_EQ(NewRegExpSource(), Parse(LexerMode::RegExp));

  PrepareSouceCode("/\\//");
  EXPECT_EQ(NewRegExpSource(), Parse(LexerMode::RegExp));

  PrepareSouceCode("/foo");
  EXPECT_EQ(NewRegExpSource() + NewError(ERROR_REGEXP_EXPECT_SLASH, 0, 4),
            Parse(LexerMode::RegExp))
      << "No slash";

  PrepareSouceCode("/[ab/");
  EXPECT_EQ(NewRegExpSource() + NewError(ERROR_REGEXP_EXPECT_SLASH, 0, 5),
            Parse(LexerMode::RegExp))
      << "Open bracket";
}

TEST_F(LexerTest, StringLiteral) {
  PrepareSouceCode("\"\"");
  EXPECT_EQ(NewStringLiteral(""), Parse());

  PrepareSouceCode("''");
  EXPECT_EQ(NewStringLiteral(""), Parse());

  PrepareSouceCode("'abc'");
  EXPECT_EQ(NewStringLiteral("abc"), Parse());

  PrepareSouceCode("'\\b\\t\\n\\v\\f\\r\\\\\\''");
  EXPECT_EQ(NewStringLiteral("\b\t\n\v\f\r\\'"), Parse());

  PrepareSouceCode("'\\0\\x12\\xAB'");
  EXPECT_EQ(NewStringLiteral({0x00, 0x12, 0xAB}), Parse());

  PrepareSouceCode("'\\u1234'");
  EXPECT_EQ(NewStringLiteral({0x1234}), Parse());

  PrepareSouceCode("'\\u{123}'");
  EXPECT_EQ(NewStringLiteral({0x123}), Parse());
}

TEST_F(LexerTest, StringLiteralError) {
  PrepareSouceCode("'\n'");
  EXPECT_EQ(
      NewStringLiteral("\n") + NewError(ERROR_STRING_LITERAL_NEWLINE, 0, 1),
      Parse())
      << "String literal can not contain newline";

  PrepareSouceCode("'ab\n'");
  EXPECT_EQ(
      NewStringLiteral("ab\n") + NewError(ERROR_STRING_LITERAL_NEWLINE, 0, 3),
      Parse())
      << "String literal can not contain newline";

  PrepareSouceCode("'ab\ncd'");
  EXPECT_EQ(
      NewStringLiteral("ab\ncd") + NewError(ERROR_STRING_LITERAL_NEWLINE, 0, 3),
      Parse())
      << "String literal can not contain newline";

  PrepareSouceCode("'foo");
  EXPECT_EQ(NewStringLiteral(0, 4, "foo") +
                NewError(ERROR_STRING_LITERAL_NOT_CLOSED, 0, 4),
            Parse())
      << "No closing double quote";

  PrepareSouceCode("'\\x");
  EXPECT_EQ(NewStringLiteral(0, 3, "") +
                NewError(ERROR_STRING_LITERAL_NOT_CLOSED, 0, 3),
            Parse())
      << "No hexadecimal digit after '\\x'";

  PrepareSouceCode("'\\a'");
  EXPECT_EQ(
      NewStringLiteral("a") + NewError(ERROR_STRING_LITERAL_BACKSLASH, 1, 3),
      Parse(ParserOptions::Builder().set_enable_strict_backslash(true).Build()))
      << "'\\a' is not backslash escape sequence";

  PrepareSouceCode("'\\xY'");
  EXPECT_EQ(NewStringLiteral("") +
                NewError(ERROR_STRING_LITERAL_BACKSLASH_HEX_DIGIT, 1, 3),
            Parse())
      << "'\\x' without hexadecimal digit";

  PrepareSouceCode("'\\uX'");
  EXPECT_EQ(NewStringLiteral("") +
                NewError(ERROR_STRING_LITERAL_BACKSLASH_HEX_DIGIT, 1, 3),
            Parse())
      << "'\\u' without hexadecimal digit";

  PrepareSouceCode("'\\u{X'");
  EXPECT_EQ(NewStringLiteral("") +
                NewError(ERROR_STRING_LITERAL_BACKSLASH_HEX_DIGIT, 1, 4),
            Parse())
      << "'\\u' without hexadecimal digit";
}

}  // namespace parser
}  // namespace joana

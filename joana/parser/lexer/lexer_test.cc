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
#include "joana/parser/public/parse.h"
#include "joana/parser/simple_error_sink.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/edit_context_builder.h"
#include "joana/public/ast/error_codes.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/error_sink.h"
#include "joana/public/memory/zone.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_range.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace internal {

namespace {

enum ErrorCode {
  NONE = ast::kLexerErrorCodeBase,
#define V(token, reason) ERROR_##token##_##reason,
  FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
};

std::string ToString(const ast::Node& node) {
  std::ostringstream ostream;
  node.PrintTo(&ostream);
  return ostream.str();
}

}  // namespace

class LexerTest : public ::testing::Test {
 protected:
  LexerTest();

  ast::NodeFactory& factory() { return node_factory_; }
  const ParserOptions& options() const { return options_; }

  SourceCodeRange MakeRange(int start, int end) const;
  SourceCodeRange MakeRange() const;
  std::string NewComment(int start, int end);
  std::string NewComment();
  std::string NewError(ErrorCode error_code, int start, int end);
  std::string NewInvalid(int start,
                         int error_position,
                         int end,
                         int error_code);
  std::string NewName(int start, int end);
  std::string NewNumericLiteral(int start, int end, double value);
  std::string NewNumericLiteral(double value);
  std::string NewPunctuator(ast::PunctuatorKind kind);
  std::string NewStringLiteral(base::StringPiece data);
  std::string NewStringLiteral(const std::vector<base::char16> data);
  void PrepareSouceCode(base::StringPiece script_text);
  std::string Parse();

 private:
  SimpleErrorSink error_sink_;
  Zone zone_;
  ast::NodeFactory node_factory_;
  std::unique_ptr<ast::EditContext> context_;
  ParserOptions options_;
  const SourceCode* source_code_ = nullptr;
  SourceCode::Factory source_code_factory_;

  DISALLOW_COPY_AND_ASSIGN(LexerTest);
};

LexerTest::LexerTest()
    : zone_("LexerTest"),
      node_factory_(&zone_),
      context_(ast::EditContext::Builder()
                   .SetErrorSink(&error_sink_)
                   .SetNodeFactory(&node_factory_)
                   .Build()),
      source_code_factory_(&zone_) {}

SourceCodeRange LexerTest::MakeRange(int start, int end) const {
  DCHECK(source_code_);
  return source_code_->Slice(start, end);
}

SourceCodeRange LexerTest::MakeRange() const {
  DCHECK(source_code_);
  return source_code_->range();
}

std::string LexerTest::NewComment(int start, int end) {
  return ToString(factory().NewComment(MakeRange(start, end)));
}

std::string LexerTest::NewComment() {
  return ToString(factory().NewComment(MakeRange()));
}

std::string LexerTest::NewError(ErrorCode error_code, int start, int end) {
  std::ostringstream ostream;
  ostream << ' ' << error_code << '@' << source_code_->Slice(start, end);
  return ostream.str();
}

std::string LexerTest::NewInvalid(int start,
                                  int error_offset,
                                  int end,
                                  int error_code) {
  std::ostringstream ostream;
  factory()
      .NewInvalid(source_code_->Slice(start, end), error_code)
      .PrintTo(&ostream);
  ostream << ' ' << error_code << '@' << MakeRange(start, error_offset);
  return ostream.str();
}

std::string LexerTest::NewName(int start, int end) {
  return ToString(factory().NewName(source_code_->Slice(start, end)));
}

std::string LexerTest::NewNumericLiteral(int start, int end, double value) {
  return ToString(factory().NewNumericLiteral(MakeRange(start, end), value));
}

std::string LexerTest::NewNumericLiteral(double value) {
  return ToString(factory().NewNumericLiteral(MakeRange(), value));
}

std::string LexerTest::NewPunctuator(ast::PunctuatorKind kind) {
  return ToString(factory().NewPunctuator(MakeRange(), kind));
}

std::string LexerTest::NewStringLiteral(base::StringPiece data8) {
  const auto& data16 = base::UTF8ToUTF16(data8);
  return ToString(
      factory().NewStringLiteral(MakeRange(), base::StringPiece16(data16)));
}

std::string LexerTest::NewStringLiteral(const std::vector<base::char16> data) {
  return ToString(factory().NewStringLiteral(
      MakeRange(), base::StringPiece16(data.data(), data.size())));
}

void LexerTest::PrepareSouceCode(base::StringPiece script_text) {
  const auto& script_text16 = base::UTF8ToUTF16(script_text);
  source_code_ = &source_code_factory_.New(base::FilePath(),
                                           base::StringPiece16(script_text16));
  error_sink_.Reset();
}

std::string LexerTest::Parse() {
  DCHECK(source_code_);
  Lexer lexer(context_.get(), source_code_->range(), options());
  std::ostringstream ostream;
  auto delimiter = "";
  while (lexer.CanPeekToken()) {
    ostream << delimiter;
    delimiter = " ";
    auto& node = lexer.PeekToken();
    node.PrintTo(&ostream);
    lexer.Advance();
  }
  for (const auto* error : error_sink_.errors()) {
    ostream << ' ' << error->error_code() << '@' << error->range();
  }
  return ostream.str();
}

TEST_F(LexerTest, BlockComment) {
  PrepareSouceCode("/**/");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* abc */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/** */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* * */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* ** */");
  EXPECT_EQ(NewComment(), Parse());

  PrepareSouceCode("/* ***/");
  EXPECT_EQ(NewComment(), Parse());
}

TEST_F(LexerTest, Empty) {
  PrepareSouceCode("");
  EXPECT_EQ("", Parse());
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
  EXPECT_EQ(static_cast<int>(ast::NameId::While),
            factory().NewName(MakeRange(0, 5)).number())
      << "We can identify keyword 'while'.";

  PrepareSouceCode("while");
  EXPECT_TRUE(factory().NewName(MakeRange(0, 5)).IsKeyword());

  PrepareSouceCode("from");
  EXPECT_EQ(static_cast<int>(ast::NameId::From),
            factory().NewName(MakeRange(0, 4)).number())
      << "We can identify contextual keyword 'from'.";

  PrepareSouceCode("from");
  EXPECT_FALSE(factory().NewName(MakeRange(0, 4)).IsKeyword());

  PrepareSouceCode("of");
  EXPECT_EQ(static_cast<int>(ast::NameId::Of),
            factory().NewName(MakeRange(0, 2)).number())
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
}

TEST_F(LexerTest, NumericLiteralError) {
  PrepareSouceCode("0b1034");
  EXPECT_EQ(NewInvalid(0, 5, 6, ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT),
            Parse())
      << "Binary literal contains 3 and 4";

  PrepareSouceCode("0o128934");
  EXPECT_EQ(NewInvalid(0, 5, 8, ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT),
            Parse())
      << "Octal literal contains 8 and 9";

  PrepareSouceCode("0x12X34");
  EXPECT_EQ(NewInvalid(0, 5, 7, ERROR_NUMERIC_LITERAL_INTEGER_BAD_DIGIT),
            Parse())
      << "Hexadecimal literal contains 'X'";

  PrepareSouceCode("0x");
  EXPECT_EQ(NewInvalid(0, 2, 2, ERROR_NUMERIC_LITERAL_INTEGER_NO_DIGITS),
            Parse())
      << "No digits after '0x'";

  PrepareSouceCode("0123");
  EXPECT_EQ(NewInvalid(0, 4, 4, ERROR_NUMERIC_LITERAL_INTEGER_OCTAL), Parse())
      << "Strict mode does not allow legacy octal literal";

  PrepareSouceCode("0f0");
  EXPECT_EQ(NewInvalid(0, 2, 3, ERROR_NUMERIC_LITERAL_DECIMAL_BAD_DIGIT),
            Parse());

  PrepareSouceCode("12x43");
  EXPECT_EQ(NewInvalid(0, 3, 5, ERROR_NUMERIC_LITERAL_DECIMAL_BAD_DIGIT),
            Parse())
      << "Decimal contains letter.";
}

TEST_F(LexerTest, Punctuator) {
  // ";"
  PrepareSouceCode(";");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // ":"
  PrepareSouceCode(":");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Colon), Parse());

  // "=>"
  PrepareSouceCode("=>");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Arrow), Parse());

  // "("
  PrepareSouceCode("(");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // ")"
  PrepareSouceCode(")");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // "?"
  PrepareSouceCode("?");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Question), Parse());

  // "~"
  PrepareSouceCode("~");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::BitNot), Parse());

  // "["
  PrepareSouceCode("[");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // "]"
  PrepareSouceCode("]");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // "{"
  PrepareSouceCode("{");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // "}"
  PrepareSouceCode("}");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Semicolon), Parse());

  // "."
  PrepareSouceCode(".");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Dot), Parse());

  PrepareSouceCode("...");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::DotDotDot), Parse());

  // "!"
  PrepareSouceCode("!");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::LogicalNot), Parse());

  PrepareSouceCode("!=");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::NotEqual), Parse());

  PrepareSouceCode("!==");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::NotEqualEqual), Parse());

  // "+"
  PrepareSouceCode("+");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::Plus), Parse());

  PrepareSouceCode("++");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::PlusPlus), Parse());

  PrepareSouceCode("+=");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::PlusPlus), Parse());

  // "<"
  PrepareSouceCode("<");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::LessThan), Parse());

  PrepareSouceCode("<<");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::LeftShift), Parse());

  PrepareSouceCode("<=");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::LessThanOrEqual), Parse());

  // ">"
  PrepareSouceCode(">");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::GreaterThan), Parse());

  PrepareSouceCode(">>");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::RightShift), Parse());

  PrepareSouceCode(">>>");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::UnsignedRightShift), Parse());

  PrepareSouceCode(">=");
  EXPECT_EQ(NewPunctuator(ast::PunctuatorKind::GreaterThanOrEqual), Parse());
}

TEST_F(LexerTest, PunctuatorError) {
  PrepareSouceCode("..");
  EXPECT_EQ(NewInvalid(0, 2, 2, ERROR_PUNCTUATOR_DOT_DOT), Parse())
      << "'..' is not a valid punctuator.";
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
  EXPECT_EQ(NewInvalid(0, 4, 4, ERROR_STRING_LITERAL_NOT_CLOSED), Parse())
      << "No closing double quote";

  PrepareSouceCode("'\\x");
  EXPECT_EQ(NewInvalid(0, 3, 3, ERROR_STRING_LITERAL_NOT_CLOSED), Parse())
      << "No hexadecimal digit after '\\x'";

  PrepareSouceCode("'\\a'");
  EXPECT_EQ(
      NewStringLiteral("a") + NewError(ERROR_STRING_LITERAL_BACKSLASH, 1, 3),
      Parse())
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

}  // namespace internal
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "joana/parser/type/type_lexer.h"

#include "base/macros.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/parser/type/type_error_codes.h"
#include "joana/testing/lexer_test_base.h"

namespace joana {
namespace parser {

namespace {

std::string ToStringFromVector(const std::vector<const ast::Token*>& tokens) {
  std::ostringstream ostream;
  for (const auto& token : tokens) {
    ostream << token << std::endl;
  }
  return ostream.str();
}

template <typename... Parameters>
std::string ToString(const Parameters&... tokens) {
  return ToStringFromVector({&tokens...});
}

}  // namespace

//
// TypeLexerTest
//
class TypeLexerTest : public LexerTestBase {
 protected:
  TypeLexerTest() = default;
  ~TypeLexerTest() override = default;

  TypeLexerMode mode() const { return lexer_mode_; }
  void set_lexer_mode(TypeLexerMode mode) { lexer_mode_ = mode; }

  std::string NewError(int start, int end, TypeErrorCode error_code);
  const ast::Token& NewName(int start, int end);
  const ast::Token& NewPunctuator(int start, int end, ast::PunctuatorKind kind);

  std::string ScanToString(const ParserOptions& options);
  std::string ScanToString();

 private:
  TypeLexerMode lexer_mode_ = TypeLexerMode::Normal;

  DISALLOW_COPY_AND_ASSIGN(TypeLexerTest);
};

std::string TypeLexerTest::NewError(int start,
                                    int end,
                                    TypeErrorCode error_code) {
  std::ostringstream ostream;
  ostream << ' ' << static_cast<int>(error_code) << '@'
          << source_code().Slice(start, end);
  return ostream.str();
}

const ast::Token& TypeLexerTest::NewName(int start, int end) {
  return node_factory().NewName(source_code().Slice(start, end));
}

const ast::Token& TypeLexerTest::NewPunctuator(int start,
                                               int end,
                                               ast::PunctuatorKind kind) {
  return node_factory().NewPunctuator(source_code().Slice(start, end), kind);
}

std::string TypeLexerTest::ScanToString(const ParserOptions& options) {
  TypeLexer lexer(&context(), source_code().range(), options, lexer_mode_);
  std::ostringstream ostream;
  while (lexer.CanPeekToken())
    ostream << lexer.ConsumeToken() << std::endl;
  for (const auto* error : error_sink().errors()) {
    ostream << ' ' << error->error_code() << '@' << error->range();
  }
  return ostream.str();
}

std::string TypeLexerTest::ScanToString() {
  return ScanToString({});
}

TEST_F(TypeLexerTest, Name) {
  PrepareSouceCode("foo(new:bar, baz=, ...quux) : number");
  EXPECT_EQ(
      ToString(
          NewName(0, 3),
          NewPunctuator(3, 4, ast::PunctuatorKind::LeftParenthesis),
          NewName(4, 7), NewPunctuator(7, 8, ast::PunctuatorKind::Colon),
          NewName(8, 11), NewPunctuator(11, 12, ast::PunctuatorKind::Comma),
          NewName(13, 16), NewPunctuator(16, 17, ast::PunctuatorKind::Equal),
          NewPunctuator(17, 18, ast::PunctuatorKind::Comma),
          NewPunctuator(19, 22, ast::PunctuatorKind::DotDotDot),
          NewName(22, 26),
          NewPunctuator(26, 27, ast::PunctuatorKind::RightParenthesis),
          NewPunctuator(28, 29, ast::PunctuatorKind::Colon), NewName(30, 36)),
      ScanToString());
}

TEST_F(TypeLexerTest, Errors) {
  PrepareSouceCode("..");
  EXPECT_EQ(ToString(NewPunctuator(0, 2, ast::PunctuatorKind::Invalid)) +
                NewError(0, 2, TypeErrorCode::ERROR_TYPE_UNEXPECT_DOT),
            ScanToString());
}

TEST_F(TypeLexerTest, JsDocMdoe) {
  set_lexer_mode(TypeLexerMode::JsDoc);
  PrepareSouceCode("\n  * foo");
  EXPECT_EQ(ToString(NewName(5, 8)), ScanToString());
}

TEST_F(TypeLexerTest, Punctuator) {
  PrepareSouceCode("!");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::LogicalNot)),
            ScanToString());

  PrepareSouceCode("( )");
  EXPECT_EQ(
      ToString(NewPunctuator(0, 1, ast::PunctuatorKind::LeftParenthesis),
               NewPunctuator(2, 3, ast::PunctuatorKind::RightParenthesis)),
      ScanToString());

  PrepareSouceCode(",");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::Comma)),
            ScanToString());

  PrepareSouceCode(":");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::Colon)),
            ScanToString());

  PrepareSouceCode("=");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::Equal)),
            ScanToString());

  PrepareSouceCode("?");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::Question)),
            ScanToString());

  PrepareSouceCode("< >");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::LessThan),
                     NewPunctuator(2, 3, ast::PunctuatorKind::GreaterThan)),
            ScanToString());

  PrepareSouceCode("[ ]");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::LeftBracket),
                     NewPunctuator(2, 3, ast::PunctuatorKind::RightBracket)),
            ScanToString());

  PrepareSouceCode("{ }");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::LeftBrace),
                     NewPunctuator(2, 3, ast::PunctuatorKind::RightBrace)),
            ScanToString());

  PrepareSouceCode(",");
  EXPECT_EQ(ToString(NewPunctuator(0, 1, ast::PunctuatorKind::Invalid)),
            ScanToString())
      << "Semicolon is not valid token in type";
}

}  // namespace parser
}  // namespace joana

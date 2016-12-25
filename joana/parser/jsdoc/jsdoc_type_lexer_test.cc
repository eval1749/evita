// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "joana/parser/jsdoc/jsdoc_type_lexer.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/parser/jsdoc/jsdoc_error_codes.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/public/parser_options.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/testing/simple_error_sink.h"
#include "testing/gtest/include/gtest/gtest.h"

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

class JsDocTypeLexerTest : public ::testing::Test {
 protected:
  JsDocTypeLexerTest();
  ~JsDocTypeLexerTest() override = default;

  std::string NewError(int start, int end, JsDocErrorCode error_code);
  const ast::Token& NewName(int start, int end);
  const ast::Token& NewPunctuator(int start, int end, ast::PunctuatorKind kind);

  void PrepareSouceCode(base::StringPiece script_text);
  std::string ScanToString(const ParserOptions& options);
  std::string ScanToString();

 private:
  SimpleErrorSink error_sink_;
  Zone zone_;
  ast::NodeFactory node_factory_;
  const std::unique_ptr<ParserContext> context_;
  const SourceCode* source_code_ = nullptr;
  SourceCode::Factory source_code_factory_;

  DISALLOW_COPY_AND_ASSIGN(JsDocTypeLexerTest);
};

JsDocTypeLexerTest::JsDocTypeLexerTest()
    : zone_("JsDocTypeLexerTest"),
      node_factory_(&zone_),
      context_(ParserContext::Builder()
                   .set_error_sink(&error_sink_)
                   .set_node_factory(&node_factory_)
                   .Build()),
      source_code_factory_(&zone_) {}

std::string JsDocTypeLexerTest::NewError(int start,
                                         int end,
                                         JsDocErrorCode error_code) {
  std::ostringstream ostream;
  ostream << ' ' << static_cast<int>(error_code) << '@'
          << source_code_->Slice(start, end);
  return ostream.str();
}

const ast::Token& JsDocTypeLexerTest::NewName(int start, int end) {
  return node_factory_.NewName(source_code_->Slice(start, end));
}

const ast::Token& JsDocTypeLexerTest::NewPunctuator(int start,
                                                    int end,
                                                    ast::PunctuatorKind kind) {
  return node_factory_.NewPunctuator(source_code_->Slice(start, end), kind);
}

void JsDocTypeLexerTest::PrepareSouceCode(base::StringPiece script_text) {
  const auto& script_text16 = base::UTF8ToUTF16(script_text);
  source_code_ = &source_code_factory_.New(base::FilePath(),
                                           base::StringPiece16(script_text16));
  error_sink_.Reset();
}

std::string JsDocTypeLexerTest::ScanToString(const ParserOptions& options) {
  DCHECK(source_code_);
  CharacterReader reader(source_code_->range());
  JsDocTypeLexer lexer(context_.get(), &reader, options);
  std::ostringstream ostream;
  while (lexer.CanPeekToken())
    ostream << lexer.ConsumeToken() << std::endl;
  for (const auto* error : error_sink_.errors()) {
    ostream << ' ' << error->error_code() << '@' << error->range();
  }
  return ostream.str();
}

std::string JsDocTypeLexerTest::ScanToString() {
  return ScanToString({});
}

TEST_F(JsDocTypeLexerTest, Name) {
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

TEST_F(JsDocTypeLexerTest, Errors) {
  PrepareSouceCode("..");
  EXPECT_EQ(ToString(NewPunctuator(0, 2, ast::PunctuatorKind::Invalid)) +
                NewError(0, 2, JsDocErrorCode::ERROR_JSDOC_UNEXPECT_DOT),
            ScanToString());
}

TEST_F(JsDocTypeLexerTest, Punctuator) {
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

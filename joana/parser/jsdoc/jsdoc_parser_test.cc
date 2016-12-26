// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/jsdoc/jsdoc_parser.h"

#include <memory>
#include <ostream>
#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/jsdoc_nodes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/node_forward.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/public/parser_options.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/print_as_tree.h"
#include "joana/testing/simple_error_sink.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace parser {

//
// JsDocParserTest
//
class JsDocParserTest : public ::testing::Test {
 protected:
  JsDocParserTest() = default;
  ~JsDocParserTest() override = default;

  std::string Parse(base::StringPiece script_text,
                    const ParserOptions& options);
  std::string Parse(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(JsDocParserTest);
};

std::string JsDocParserTest::Parse(base::StringPiece script_text,
                                   const ParserOptions& options) {
  Zone zone("JsDocParserTest");
  SimpleErrorSink error_sink;
  ast::NodeFactory node_factory(&zone);
  const auto& context = ParserContext::Builder()
                            .set_error_sink(&error_sink)
                            .set_node_factory(&node_factory)
                            .Build();
  SourceCode::Factory source_code_factory(&zone);
  const auto& script_text16 = base::UTF8ToUTF16(script_text);
  auto& source_code = source_code_factory.New(
      base::FilePath(), base::StringPiece16(script_text16));

  JsDocParser parser(context.get(), source_code.range(), options);
  const auto* maybe_document = parser.Parse();
  if (!maybe_document)
    return "";
  const auto& document = *maybe_document;
  std::ostringstream ostream;
  ostream << AsPrintableTree(document);
  for (const auto& error : error_sink.errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string JsDocParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

TEST_F(JsDocParserTest, Empty) {
  EXPECT_EQ("", Parse(""));
}

TEST_F(JsDocParserTest, NoTags) {
  EXPECT_EQ("", Parse("foo"));
}

TEST_F(JsDocParserTest, SyntaxDescription) {
  EXPECT_EQ(
      "@deprecated\n"
      "+--|foo bar|\n",
      Parse("@deprecated foo bar"));
}

TEST_F(JsDocParserTest, SyntaxNameList) {
  EXPECT_EQ(
      "@suppress\n"
      "+--foo\n"
      "+--bar\n"
      "+--baz\n",
      Parse("@suppress {foo, bar, baz}"));
}

TEST_F(JsDocParserTest, SyntaxNames) {
  EXPECT_EQ(
      "@template\n"
      "+--T\n",
      Parse("@template T desc"));
  EXPECT_EQ(
      "@template\n"
      "+--KEY\n"
      "+--VALUE\n",
      Parse("@template KEY, VALUE desc"));
}

TEST_F(JsDocParserTest, SyntaxNone) {
  EXPECT_EQ("@externs\n", Parse("@externs"));
}

TEST_F(JsDocParserTest, SyntaxOptionalType) {
  EXPECT_EQ("@const\n", Parse("@const desc"));
  EXPECT_EQ(
      "@const\n"
      "+--number\n",
      Parse("@const {number}"));
}

TEST_F(JsDocParserTest, SyntaxType) {
  EXPECT_EQ(
      "@enum\n"
      "+--number\n",
      Parse("@enum {number}"));
}

TEST_F(JsDocParserTest, SyntaxTypeName) {
  EXPECT_EQ(
      "@define\n"
      "+--{age:number, sex:string}\n"
      "+--|human|\n",
      Parse("@define {{age:number, sex:string}} human"));
}

TEST_F(JsDocParserTest, SyntaxTypeNameDescription) {
  EXPECT_EQ(
      "@param\n"
      "+--string\n"
      "+--foo\n"
      "+--||\n",
      Parse("@param {string} foo"));
  EXPECT_EQ(
      "@param\n"
      "+--string\n"
      "+--foo\n"
      "+--|desc1|\n"
      "@param\n"
      "+--number\n"
      "+--bar\n"
      "+--||\n",
      Parse("@param {string} foo desc1\n"
            "@param {number} bar\n"));
}

}  // namespace parser
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/jsdoc/jsdoc_parser.h"

#include <sstream>
#include <string>

#include "joana/ast/jsdoc_nodes.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/public/parser_options.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace parser {

//
// JsDocParserTest
//
class JsDocParserTest : public LexerTestBase {
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
  PrepareSouceCode(script_text);
  JsDocParser parser(&context(), source_code().range(), options);
  std::ostringstream ostream;
  if (auto* document = parser.Parse())
    ostream << AsPrintableTree(*document) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string JsDocParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

TEST_F(JsDocParserTest, Empty) {
  EXPECT_EQ("", Parse(""));
}

TEST_F(JsDocParserTest, InlineTag) {
  EXPECT_EQ(
      "#document\n"
      "+--|foo {@code bar}|\n"
      "+--@const\n",
      Parse("foo {@code bar} @const"));

  EXPECT_EQ("JSDOC_ERROR_JSDOC_EXPECT_RBRACE@4:14\n", Parse("foo {@code bar"))
      << "Open inline tag";
}

TEST_F(JsDocParserTest, MultipleLines) {
  EXPECT_EQ(
      "#document\n"
      "+--|*|\n"
      "+--@typedef\n"
      "|  +--#type union\n"
      "|  |  +--foo\n"
      "|  |  +--bar\n",
      Parse(" * @typedef {\n"
            " *     foo|\n"
            " *     bar}\n"));
}

TEST_F(JsDocParserTest, NoTags) {
  EXPECT_EQ("", Parse("foo"));
}

TEST_F(JsDocParserTest, SyntaxDescription) {
  EXPECT_EQ(
      "#document\n"
      "+--@deprecated\n"
      "|  +--|foo bar|\n",
      Parse("@deprecated foo bar"));
}

TEST_F(JsDocParserTest, SyntaxNameList) {
  EXPECT_EQ(
      "#document\n"
      "+--@suppress\n"
      "|  +--#name foo\n"
      "|  +--#name bar\n"
      "|  +--#name baz\n",
      Parse("@suppress {foo, bar, baz}"));
}

TEST_F(JsDocParserTest, SyntaxNames) {
  EXPECT_EQ(
      "#document\n"
      "+--@template\n"
      "|  +--#name T\n"
      "+--|desc|\n",
      Parse("@template T desc"));
  EXPECT_EQ(
      "#document\n"
      "+--@template\n"
      "|  +--#name KEY\n"
      "|  +--#name VALUE\n"
      "+--|desc|\n",
      Parse("@template KEY, VALUE desc"));
}

TEST_F(JsDocParserTest, SyntaxNone) {
  EXPECT_EQ(
      "#document\n"
      "+--@externs\n",
      Parse("@externs"));
}

TEST_F(JsDocParserTest, SyntaxOptionalType) {
  EXPECT_EQ(
      "#document\n"
      "+--@const\n"
      "+--|desc|\n",
      Parse("@const desc"));
  EXPECT_EQ(
      "#document\n"
      "+--@const\n"
      "|  +--#type number\n",
      Parse("@const {number}"));
}

TEST_F(JsDocParserTest, SyntaxSingleLine) {
  EXPECT_EQ(
      "#document\n"
      "+--@author\n"
      "|  +--|foo|\n"
      "+--|bar|\n",
      Parse("@author foo  \nbar"))
      << "The parameter of @author should not have leading and trailing "
         "whitespaces.";
}

TEST_F(JsDocParserTest, SyntaxType) {
  EXPECT_EQ(
      "#document\n"
      "+--@enum\n"
      "|  +--#type number\n",
      Parse("@enum {number}"));
}

TEST_F(JsDocParserTest, SyntaxTypeDescription) {
  EXPECT_EQ(
      "#document\n"
      "+--@define\n"
      "|  +--#type record\n"
      "|  |  +--age: number\n"
      "|  |  +--sex: string\n"
      "|  +--|human|\n",
      Parse("@define {{age:number, sex:string}} human"));
}

TEST_F(JsDocParserTest, SyntaxTypeNameDescription) {
  EXPECT_EQ(
      "#document\n"
      "+--@param\n"
      "|  +--#type string\n"
      "|  +--#name foo\n"
      "|  +--||\n",
      Parse("@param {string} foo"));
  EXPECT_EQ(
      "#document\n"
      "+--@param\n"
      "|  +--#type string\n"
      "|  +--#name foo\n"
      "|  +--|desc1|\n"
      "+--@param\n"
      "|  +--#type number\n"
      "|  +--#name bar\n"
      "|  +--||\n",
      Parse("@param {string} foo desc1\n"
            "@param {number} bar\n"));
}

}  // namespace parser
}  // namespace joana

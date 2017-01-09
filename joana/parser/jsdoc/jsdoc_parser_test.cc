// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/jsdoc/jsdoc_parser.h"

#include <sstream>
#include <string>

#include "joana/ast/jsdoc_syntaxes.h"
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
      "JsDocDocument\n"
      "+--JsDocText |foo {@code bar}|\n"
      "+--JsDocTag\n"
      "|  +--Name |@const|\n",
      Parse("foo {@code bar} @const"));

  EXPECT_EQ("JSDOC_ERROR_TAG_EXPECT_RBRACE@4:14\n", Parse("foo {@code bar"))
      << "Open inline tag";
}

TEST_F(JsDocParserTest, MultipleLines) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocText |*|\n"
      "+--JsDocTag\n"
      "|  +--Name |@typedef|\n"
      "|  +--UnionType\n"
      "|  |  +--TypeName\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--TypeName\n"
      "|  |  |  +--Name |bar|\n",
      Parse(" * @typedef {\n"
            " *     foo|\n"
            " *     bar}\n"));
}

TEST_F(JsDocParserTest, NoTags) {
  EXPECT_EQ("", Parse("foo"));
}

TEST_F(JsDocParserTest, NotBlockTags) {
  EXPECT_EQ("", Parse("Ctrl+@foo"));
}

TEST_F(JsDocParserTest, SyntaxDescription) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@deprecated|\n"
      "|  +--JsDocText |foo bar|\n",
      Parse("@deprecated foo bar"));
}

TEST_F(JsDocParserTest, SyntaxModifies) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@modifies|\n"
      "|  +--Name |this|\n",
      Parse("@modifies { this }"));
}

TEST_F(JsDocParserTest, SyntaxModifiesError) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@modifies|\n"
      "|  +--Name ||\n"
      "JSDOC_ERROR_TAG_EXPECT_LBRACE@0:9\n"
      "JSDOC_ERROR_TAG_EXPECT_ARGUMENTS_OR_THIS@0:9\n"
      "JSDOC_ERROR_TAG_EXPECT_RBRACE@0:9\n",
      Parse("@modifies"));

  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@modifies|\n"
      "|  +--Name |this|\n"
      "JSDOC_ERROR_TAG_EXPECT_LBRACE@0:10\n"
      "JSDOC_ERROR_TAG_EXPECT_RBRACE@0:14\n",
      Parse("@modifies this"));

  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@modifies|\n"
      "|  +--Name |foo|\n"
      "JSDOC_ERROR_TAG_EXPECT_ARGUMENTS_OR_THIS@0:15\n",
      Parse("@modifies { foo}"));
}

TEST_F(JsDocParserTest, SyntaxNameList) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@suppress|\n"
      "|  +--Name |foo|\n"
      "|  +--Name |bar|\n"
      "|  +--Name |baz|\n",
      Parse("@suppress {foo, bar, baz}"));
}

TEST_F(JsDocParserTest, SyntaxNames) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@template|\n"
      "|  +--Name |T|\n"
      "+--JsDocText |desc|\n",
      Parse("@template T desc"));

  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@template|\n"
      "|  +--Name |KEY|\n"
      "|  +--Name |VALUE|\n"
      "+--JsDocText |desc|\n",
      Parse("@template KEY, VALUE desc"));
}

TEST_F(JsDocParserTest, SyntaxNamesError) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@template|\n"
      "JSDOC_ERROR_TAG_EXPECT_NAMES@0:9\n",
      Parse("@template"))
      << "@template should have at least one name.";
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@template|\n"
      "+--JsDocText |123|\n"
      "JSDOC_ERROR_TAG_EXPECT_NAME@0:10\n"
      "JSDOC_ERROR_TAG_EXPECT_NAMES@0:10\n",
      Parse("@template 123"))
      << "'123' is not name.";
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@template|\n"
      "|  +--Name |foo|\n"
      "+--JsDocText |123|\n"
      "JSDOC_ERROR_TAG_EXPECT_NAME@0:15\n",
      Parse("@template foo, 123"))
      << "'123' is not name.";
}

TEST_F(JsDocParserTest, SyntaxNone) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@externs|\n",
      Parse("@externs"));
}

TEST_F(JsDocParserTest, SyntaxOptionalType) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@const|\n"
      "+--JsDocText |desc|\n",
      Parse("@const desc"));

  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@const|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |number|\n",
      Parse("@const {number}"));
}

TEST_F(JsDocParserTest, SyntaxSingleLine) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@author|\n"
      "|  +--JsDocText |foo|\n"
      "+--JsDocText |bar|\n",
      Parse("@author foo  \nbar"))
      << "The parameter of @author should not have leading and trailing "
         "whitespaces.";
}

TEST_F(JsDocParserTest, SyntaxType) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@enum|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |number|\n",
      Parse("@enum {number}"));
}

TEST_F(JsDocParserTest, SyntaxTypeDescription) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@define|\n"
      "|  +--RecordType\n"
      "|  |  +--Property\n"
      "|  |  |  +--Name |age|\n"
      "|  |  |  +--TypeName\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  +--Property\n"
      "|  |  |  +--Name |sex|\n"
      "|  |  |  +--TypeName\n"
      "|  |  |  |  +--Name |string|\n"
      "|  +--JsDocText |human|\n",
      Parse("@define {{age:number, sex:string}} human"));
}

TEST_F(JsDocParserTest, SyntaxTypeVarDescription) {
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@param|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |string|\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--JsDocText ||\n",
      Parse("@param {string} foo"));
  EXPECT_EQ(
      "JsDocDocument\n"
      "+--JsDocTag\n"
      "|  +--Name |@param|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |string|\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--JsDocText |desc1|\n"
      "+--JsDocTag\n"
      "|  +--Name |@param|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |number|\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n"
      "|  +--JsDocText ||\n",
      Parse("@param {string} foo desc1\n"
            "@param {number} bar\n"));
}

}  // namespace parser
}  // namespace joana

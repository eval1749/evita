// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/type/type_parser.h"

#include <sstream>
#include <string>

#include "joana/ast/types.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/public/parser_options.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace parser {

class TypeParserTest : public LexerTestBase {
 protected:
  TypeParserTest() = default;
  ~TypeParserTest() override = default;

  std::string Parse(base::StringPiece script_text,
                    const ParserOptions& options);
  std::string Parse(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeParserTest);
};

std::string TypeParserTest::Parse(base::StringPiece script_text,
                                  const ParserOptions& options) {
  PrepareSouceCode(script_text);
  TypeParser parser(&context(), source_code().range(), options);
  const auto& type = parser.Parse();

  std::ostringstream ostream;
  ostream << AsPrintableTree(type) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string TypeParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

TEST_F(TypeParserTest, AnyType) {
  EXPECT_EQ("AnyType |*|\n", Parse("*"));
}

TEST_F(TypeParserTest, Empty) {
  EXPECT_EQ(
      "InvalidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@0:0\n",
      Parse(""));
}

TEST_F(TypeParserTest, FunctionType) {
  EXPECT_EQ(
      "TypeName\n"
      "+--Name |function|\n",
      Parse("function"));

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |baz|\n"
      "+--VoidType ||\n",
      Parse("function(bar, baz)"));

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |baz|\n"
      "+--TypeName\n"
      "|  +--Name |quux|\n",
      Parse("function(bar, baz):quux"));

  EXPECT_EQ(
      "FunctionType<New>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |baz|\n"
      "+--TypeName\n"
      "|  +--Name |quux|\n",
      Parse("function(new:bar, baz):quux"));

  EXPECT_EQ(
      "FunctionType<This>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |baz|\n"
      "+--TypeName\n"
      "|  +--Name |quux|\n",
      Parse("function(this:bar, baz):quux"));
}

TEST_F(TypeParserTest, FunctionTypeError) {
  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |foo|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:12\n",
      Parse("function(foo"));

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |foo|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@13:13\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(foo,"));

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |foo|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@13:16\n",
      Parse("function(foo bar)"))
      << "No comma between types";

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |foo|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@13:16\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:16\n",
      Parse("function(foo bar"))
      << "No comma between types and not close";

  EXPECT_EQ(
      "FunctionType<Normal>\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |foo|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@12:13\n"
      "TYPE_ERROR_TYPE_UNEXPECT_RBRACE@8:13\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(foo}"))
      << "Mismatched bracket";

  EXPECT_EQ(
      "FunctionType<New>\n"
      "+--Tuple |(new|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_COLON@12:12\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@12:12\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:12\n",
      Parse("function(new"))
      << "No colon after 'new'";

  EXPECT_EQ(
      "FunctionType<New>\n"
      "+--Tuple |(new:|\n"
      "+--VoidType ||\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@13:13\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(new:"))
      << "No type after new:'";
}

TEST_F(TypeParserTest, NullableType) {
  EXPECT_EQ(
      "NullableType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n",
      Parse("?foo"));
}

TEST_F(TypeParserTest, NonNullableType) {
  EXPECT_EQ(
      "NonNullableType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n",
      Parse("!foo"));
}

TEST_F(TypeParserTest, OptionalType) {
  EXPECT_EQ(
      "OptionalType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n",
      Parse("foo="));
}

TEST_F(TypeParserTest, RecordType) {
  EXPECT_EQ(
      "RecordType\n"
      "+--Property\n"
      "|  +--Name |done|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |boolean|\n"
      "+--Property\n"
      "|  +--Name |next|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |T|\n",
      Parse("{done: boolean, next: T}"));
}

TEST_F(TypeParserTest, RecordTypeError) {
  EXPECT_EQ(
      "RecordType |{|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:1\n",
      Parse("{"));

  EXPECT_EQ(
      "RecordType |{foo|\n"
      "TYPE_ERROR_TYPE_EXPECT_COLON@4:4\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:4\n",
      Parse("{foo"));

  EXPECT_EQ(
      "RecordType\n"
      "+--Property\n"
      "|  +--Name |foo|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:9\n",
      Parse("{foo: bar"));

  EXPECT_EQ(
      "RecordType\n"
      "+--Property\n"
      "|  +--Name |foo|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:10\n",
      Parse("{foo: bar,"));
}

TEST_F(TypeParserTest, RestType) {
  EXPECT_EQ(
      "RestType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n",
      Parse("...foo\n"));
}

TEST_F(TypeParserTest, TupleType) {
  EXPECT_EQ(
      "TupleType\n"
      "+--TypeName\n"
      "|  +--Name |Key|\n"
      "+--TypeName\n"
      "|  +--Name |Value|\n",
      Parse("[Key, Value]"));
}

TEST_F(TypeParserTest, TupleTypeError) {
  EXPECT_EQ(
      "TupleType |[|\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@1:1\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:1\n",
      Parse("["));

  EXPECT_EQ(
      "TupleType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:4\n",
      Parse("[foo"));

  EXPECT_EQ(
      "TupleType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:4\n",
      Parse("[foo"));

  EXPECT_EQ(
      "TupleType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:5\n",
      Parse("[foo,"))
      << "No type after comma";

  EXPECT_EQ(
      "TupleType\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--TypeName\n"
      "|  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@5:8\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:8\n",
      Parse("[foo bar"))
      << "No comma between members";
}

TEST_F(TypeParserTest, TypeApplication) {
  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |Map|\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |K|\n"
      "|  +--TypeName\n"
      "|  |  +--Name |V|\n",
      Parse("Map<K, V>"));
}

TEST_F(TypeParserTest, TypeApplicationError) {
  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--Tuple |<|\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:4\n",
      Parse("foo<"));

  // Parser doesn't report error for no type parameters for type application.
  // Later pass will report error.
  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--Tuple |<>|\n",
      Parse("foo<>"))
      << "No type parameters";

  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:7\n",
      Parse("foo<bar"));

  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:8\n",
      Parse("foo<bar,"));

  EXPECT_EQ(
      "TypeApplication\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "+--Tuple\n"
      "|  +--TypeName\n"
      "|  |  +--Name |bar|\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:11\n",
      Parse("foo<bar baz"))
      << "No comma between parameters";
}

TEST_F(TypeParserTest, TypeGroup) {
  EXPECT_EQ(
      "TypeGroup\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n",
      Parse("(foo)"));
  EXPECT_EQ(
      "OptionalType\n"
      "+--TypeGroup\n"
      "|  +--UnionType\n"
      "|  |  +--TypeName\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--TypeName\n"
      "|  |  |  +--Name |bar|\n",
      Parse("(foo|bar)="));
}

TEST_F(TypeParserTest, TypeGroupError) {
  EXPECT_EQ(
      "TypeGroup\n"
      "+--TypeName\n"
      "|  +--Name |foo|\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@0:4\n",
      Parse("(foo"))
      << "No right parenthesis";
}

TEST_F(TypeParserTest, TypeName) {
  EXPECT_EQ(
      "TypeName\n"
      "+--Name |foo|\n",
      Parse("foo"));
}

TEST_F(TypeParserTest, UnionType) {
  EXPECT_EQ(
      "UnionType\n"
      "+--TypeName\n"
      "|  +--Name |number|\n"
      "+--TypeName\n"
      "|  +--Name |string|\n",
      Parse("number|string"));
}

TEST_F(TypeParserTest, UnknownType) {
  EXPECT_EQ("UnknownType |?|\n", Parse("?"));
}

}  // namespace parser
}  // namespace joana

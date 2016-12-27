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

#define TEST_PARSER(script_text)           \
  {                                        \
    auto* const source = script_text;      \
    EXPECT_EQ(source, Parse(script_text)); \
  }

TEST_F(TypeParserTest, AnyType) {
  TEST_PARSER("*\n");
}

TEST_F(TypeParserTest, Empty) {
  EXPECT_EQ(
      "(invalid)\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@0:0\n",
      Parse(""));
}

TEST_F(TypeParserTest, ErrorFunctionType) {
  EXPECT_EQ(
      "function 0\n"
      "|  +--foo\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:12\n",
      Parse("function(foo"));

  EXPECT_EQ(
      "function 0\n"
      "|  +--foo\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@13:13\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(foo,"));

  EXPECT_EQ(
      "function 0\n"
      "|  +--foo\n"
      "|  +--bar\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@13:16\n",
      Parse("function(foo bar)"))
      << "No comma between types";

  EXPECT_EQ(
      "function 0\n"
      "|  +--foo\n"
      "|  +--bar\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@13:16\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:16\n",
      Parse("function(foo bar"))
      << "No comma between types and not close";

  EXPECT_EQ(
      "function 0\n"
      "|  +--foo\n"
      "|  +--(invalid)\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_COMMA@12:13\n"
      "TYPE_ERROR_TYPE_UNEXPECT_RBRACE@8:13\n",
      Parse("function(foo}"))
      << "Mismatched bracket";

  EXPECT_EQ(
      "function 2\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_COLON@12:12\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@12:12\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:12\n",
      Parse("function(new"))
      << "No colon after 'new'";

  EXPECT_EQ(
      "function 2\n"
      "+--void\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@13:13\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(new:"))
      << "No type after new:'";
}

TEST_F(TypeParserTest, ErrorRecordType) {
  EXPECT_EQ(
      "record\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:1\n",
      Parse("{"));

  EXPECT_EQ(
      "record\n"
      "TYPE_ERROR_TYPE_EXPECT_COLON@4:4\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:4\n",
      Parse("{foo"));

  EXPECT_EQ(
      "record\n"
      "+--foo: bar\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:9\n",
      Parse("{foo: bar"));

  EXPECT_EQ(
      "record\n"
      "+--foo: bar\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:10\n",
      Parse("{foo: bar,"));
}

TEST_F(TypeParserTest, ErrorTupleType) {
  EXPECT_EQ(
      "tuple\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:1\n",
      Parse("["));

  EXPECT_EQ(
      "tuple\n"
      "+--foo\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:4\n",
      Parse("[foo"));

  EXPECT_EQ(
      "tuple\n"
      "+--foo\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:4\n",
      Parse("[foo"));

  EXPECT_EQ(
      "tuple\n"
      "+--foo\n"
      "TYPE_ERROR_TYPE_EXPECT_TYPE@5:5\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:5\n",
      Parse("[foo,"))
      << "No type after comma";

  EXPECT_EQ(
      "tuple\n"
      "+--foo\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACKET@0:8\n",
      Parse("[foo bar"))
      << "No comma between members";
}

TEST_F(TypeParserTest, ErrorTypeApplication) {
  EXPECT_EQ(
      "application foo\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:4\n",
      Parse("foo<"));

  // Parser doesn't report error for no type parameters for type application.
  // Later pass will report error.
  EXPECT_EQ("application foo\n", Parse("foo<>")) << "No type parameters";

  EXPECT_EQ(
      "application foo\n"
      "+--bar\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:7\n",
      Parse("foo<bar"));

  EXPECT_EQ(
      "application foo\n"
      "+--bar\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:8\n",
      Parse("foo<bar,"));

  EXPECT_EQ(
      "application foo\n"
      "+--bar\n"
      "TYPE_ERROR_TYPE_EXPECT_RANGLE@3:11\n",
      Parse("foo<bar baz"))
      << "No comma between parameters";
}

TEST_F(TypeParserTest, ErrorTypeGroup) {
  EXPECT_EQ(
      "group\n"
      "+--foo\n"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@0:4\n",
      Parse("(foo"))
      << "No right parenthesis";
}

TEST_F(TypeParserTest, FunctionType) {
  TEST_PARSER("function\n");
  EXPECT_EQ(
      "function 0\n"
      "|  +--bar\n"
      "|  +--baz\n"
      "+--void\n",
      Parse("function(bar, baz)"));
  EXPECT_EQ(
      "function 0\n"
      "|  +--bar\n"
      "|  +--baz\n"
      "+--quux\n",
      Parse("function(bar, baz):quux"));
  EXPECT_EQ(
      "function 2\n"
      "|  +--bar\n"
      "|  +--baz\n"
      "+--quux\n",
      Parse("function(new:bar, baz):quux"));
  EXPECT_EQ(
      "function 1\n"
      "|  +--bar\n"
      "|  +--baz\n"
      "+--quux\n",
      Parse("function(this:bar, baz):quux"));
}

TEST_F(TypeParserTest, NullableType) {
  TEST_PARSER("?foo\n");
}

TEST_F(TypeParserTest, NonNullableType) {
  TEST_PARSER("!foo\n");
}

TEST_F(TypeParserTest, OptionalType) {
  EXPECT_EQ("=foo\n", Parse("foo=\n"));
}

TEST_F(TypeParserTest, RecordType) {
  EXPECT_EQ(
      "record\n"
      "+--done: boolean\n"
      "+--next: T\n",
      Parse("{done: boolean, next: T}"));
}

TEST_F(TypeParserTest, RestType) {
  TEST_PARSER("...foo\n");
}

TEST_F(TypeParserTest, TupleType) {
  EXPECT_EQ(
      "tuple\n"
      "+--Key\n"
      "+--Value\n",
      Parse("[Key, Value]"));
}

TEST_F(TypeParserTest, TypeApplication) {
  EXPECT_EQ(
      "application Map\n"
      "+--K\n"
      "+--V\n",
      Parse("Map<K, V>"));
}

TEST_F(TypeParserTest, TypeGroup) {
  EXPECT_EQ(
      "group\n"
      "+--foo\n",
      Parse("(foo)"));
  EXPECT_EQ(
      "=group\n"
      "+--union\n"
      "|  +--foo\n"
      "|  +--bar\n",
      Parse("(foo|bar)="));
}

TEST_F(TypeParserTest, TypeName) {
  TEST_PARSER("foo\n");
}

TEST_F(TypeParserTest, UnionType) {
  EXPECT_EQ(
      "union\n"
      "+--number\n"
      "+--string\n",
      Parse("number|string"));
}

TEST_F(TypeParserTest, UnknownType) {
  TEST_PARSER("?\n");
}

}  // namespace parser
}  // namespace joana

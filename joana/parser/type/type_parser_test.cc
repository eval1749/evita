// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/type/type_parser.h"

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/types.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/public/parser_options.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/simple_error_sink.h"
#include "joana/testing/simple_formatter.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace parser {

class TypeParserTest : public ::testing::Test {
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
  Zone zone("TypeParserTest");
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

  std::ostringstream ostream;
  TypeParser parser(context.get(), source_code.range(), options);
  const auto& type = parser.Parse();
  SimpleFormatter(&ostream).Format(type);
  for (const auto& error : error_sink.errors())
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

TEST_F(TypeParserTest, Empty) {
  TEST_PARSER("(invalid)");
}

TEST_F(TypeParserTest, ErrorFunctionType) {
  EXPECT_EQ(
      "function(foo)"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:12\n",
      Parse("function(foo"));

  EXPECT_EQ(
      "function(foo)"
      // TODO(eval1749): We should have EXPECT_TYPE
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:13\n",
      Parse("function(foo,"));

  EXPECT_EQ(
      "function(foo)"
      "TYPE_ERROR_TYPE_EXPECT_RPAREN@8:16\n",
      Parse("function(foo bar"))
      << "No comma between types";

  EXPECT_EQ(
      "function(foo)"
      "TYPE_ERROR_TYPE_UNEXPECT_RBRACE@8:13\n",
      Parse("function(foo}"))
      << "Mismatched bracket";
}

TEST_F(TypeParserTest, ErrorRecordType) {
  EXPECT_EQ(
      "{}"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:1\n",
      Parse("{"));

  EXPECT_EQ(
      "{}"
      "TYPE_ERROR_TYPE_EXPECT_COLON@4:4\n"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:4\n",
      Parse("{foo"));

  EXPECT_EQ(
      "{foo: bar}"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:9\n",
      Parse("{foo: bar"));

  EXPECT_EQ(
      "{foo: bar}"
      "TYPE_ERROR_TYPE_EXPECT_RBRACE@0:10\n",
      Parse("{foo: bar,"));
}

TEST_F(TypeParserTest, AnyType) {
  TEST_PARSER("*");
}

TEST_F(TypeParserTest, FunctionType) {
  TEST_PARSER("function");
  TEST_PARSER("function(bar, baz)");
  TEST_PARSER("function(bar, baz):quux");
  TEST_PARSER("function(new:bar, baz):quux");
  TEST_PARSER("function(this:bar, baz):quux");
}

TEST_F(TypeParserTest, NullableType) {
  TEST_PARSER("?foo");
}

TEST_F(TypeParserTest, NonNullableType) {
  TEST_PARSER("!foo");
}

TEST_F(TypeParserTest, OptionalType) {
  TEST_PARSER("foo=");
}

TEST_F(TypeParserTest, RecordType) {
  TEST_PARSER("{done: boolean, next: T}");
}

TEST_F(TypeParserTest, RestType) {
  TEST_PARSER("...foo");
}

TEST_F(TypeParserTest, TupleType) {
  TEST_PARSER("[Key, Value]");
}

TEST_F(TypeParserTest, TypeApplication) {
  TEST_PARSER("Map<K, V>");
}

TEST_F(TypeParserTest, TypeName) {
  TEST_PARSER("foo");
}

TEST_F(TypeParserTest, UnionType) {
  TEST_PARSER("number|string");
}

TEST_F(TypeParserTest, UnknownType) {
  TEST_PARSER("?");
}

}  // namespace parser
}  // namespace joana

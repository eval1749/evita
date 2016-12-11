// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "joana/parser/parser.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/parser/simple_error_sink.h"
#include "joana/parser/simple_formatter.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/edit_context_builder.h"
#include "joana/public/ast/error_codes.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/error_sink.h"
#include "joana/public/memory/zone.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_range.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace internal {

class ParserTest : public ::testing::Test {
 protected:
  ParserTest() = default;
  ~ParserTest() override = default;

  std::string Parse(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(ParserTest);
};

std::string ParserTest::Parse(base::StringPiece script_text) {
  Zone zone("ParserTest");
  SimpleErrorSink error_sink;
  ast::NodeFactory node_factory(&zone);
  const auto& context = ast::EditContext::Builder()
                            .SetErrorSink(&error_sink)
                            .SetNodeFactory(&node_factory)
                            .Build();
  SourceCode::Factory source_code_factory(&zone);
  const auto& script_text16 = base::UTF8ToUTF16(script_text);
  auto& source_code = source_code_factory.New(
      base::FilePath(), base::StringPiece16(script_text16));

  std::ostringstream ostream;
  Parser parser(context.get(), source_code.range());
  SimpleFormatter(&ostream).Format(parser.Run());
  for (const auto& error : error_sink.errors())
    ostream << error << std::endl;
  return ostream.str();
}

#define TEST_PARSER(script_text)           \
  {                                        \
    auto* const source = script_text;      \
    EXPECT_EQ(source, Parse(script_text)); \
  }

TEST_F(ParserTest, BlockStatement) {
  TEST_PARSER(
      "{\n"
      "  foo;\n"
      "  bar;\n"
      "}\n");
}

TEST_F(ParserTest, Bracket) {
  EXPECT_EQ(
      "{\n}\n"
      "PASER_ERROR_BRACKET_NOT_CLOSED@0:1\n",
      Parse("{"));
}

TEST_F(ParserTest, BreakStatement) {
  TEST_PARSER(
      "do {\n"
      "  break;\n"
      "} while (bar);\n");

  EXPECT_EQ(
      "break;\n"
      "PASER_ERROR_STATEMENT_BREAK_BAD_PLACE@0:5\n",
      Parse("break;"));
}

TEST_F(ParserTest, ContinueStatement) {
  TEST_PARSER(
      "do {\n"
      "  continue;\n"
      "} while (bar);\n");

  EXPECT_EQ(
      "continue;\n"
      "PASER_ERROR_STATEMENT_CONTINUE_BAD_PLACE@0:8\n",
      Parse("continue;"));
}

TEST_F(ParserTest, DoWhileStatement) {
  TEST_PARSER(
      "do {\n"
      "  foo;\n"
      "} while (bar);\n");
}

TEST_F(ParserTest, EmptyStatement) {
  TEST_PARSER(";\n");
}

TEST_F(ParserTest, ExpressionStatement) {
  TEST_PARSER("false;\n");
  TEST_PARSER("foo;\n");
  TEST_PARSER("null;\n");
  TEST_PARSER("true;\n");
}

TEST_F(ParserTest, IfStatement) {
  TEST_PARSER(
      "if (foo)\n"
      "  bar;\n");
  TEST_PARSER(
      "if (foo)\n"
      "  bar;\n"
      "else\n"
      "  baz;\n");
}

}  // namespace internal
}  // namespace joana

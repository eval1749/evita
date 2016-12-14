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

TEST_F(ParserTest, DoStatement) {
  TEST_PARSER(
      "do {\n"
      "  foo;\n"
      "} while (bar);\n");
}

TEST_F(ParserTest, EmptyStatement) {
  TEST_PARSER(";\n");
}

TEST_F(ParserTest, ExpressionStatementArrayLiteral) {
  TEST_PARSER("[];\n");
  TEST_PARSER("[1];\n");
  TEST_PARSER("[,];\n");
  TEST_PARSER("[,,];\n");
}

TEST_F(ParserTest, ExpressionStatementArrayLiteralSpread) {
  TEST_PARSER("[1, 2, ...x];\n");
}

TEST_F(ParserTest, ExpressionStatementArrowFunction) {
  TEST_PARSER(
      "(a) => {\n"
      "  console.log(a);\n"
      "};\n");
}

TEST_F(ParserTest, ExpressionStatementArrowFunction0) {
  TEST_PARSER("() => 1;\n");
}

TEST_F(ParserTest, ExpressionStatementArrowFunction1) {
  TEST_PARSER("x => x * 2;\n");
}

TEST_F(ParserTest, ExpressionStatementArrowFunction2) {
  TEST_PARSER("(x, y) => x * y;\n");
}

TEST_F(ParserTest, ExpressionStatementAssignment1) {
  TEST_PARSER("foo = 100;\n");
  TEST_PARSER("foo = bar + baz;\n");
}

TEST_F(ParserTest, ExpressionStatementAssignmentMultiple) {
  TEST_PARSER("foo = bar = baz;\n");
}

TEST_F(ParserTest, ExpressionStatementAssignmentOperation) {
  TEST_PARSER("foo += baz;\n");
}

TEST_F(ParserTest, ExpressionStatementBinary) {
  TEST_PARSER("1 + 2 * 3;\n");
}

TEST_F(ParserTest, ExpressionStatementBinary2) {
  TEST_PARSER("(1 + 2) * 3;\n");
}

TEST_F(ParserTest, ExpressionStatementCall0) {
  TEST_PARSER("foo();\n");
}

TEST_F(ParserTest, ExpressionStatementCall1) {
  TEST_PARSER("foo(1);\n");
}

TEST_F(ParserTest, ExpressionStatementCall2) {
  TEST_PARSER("foo(1, 2);\n");
}

TEST_F(ParserTest, ExpressionStatementCall3) {
  TEST_PARSER("foo(1, 2, 3);\n");
}

TEST_F(ParserTest, ExpressionStatementComma) {
  TEST_PARSER("foo, bar, baz;\n");
}

TEST_F(ParserTest, ExpressionStatementConditional) {
  TEST_PARSER("foo ? bar : baz;\n");
}

TEST_F(ParserTest, ExpressionStatementUnary) {
  TEST_PARSER("!foo;\n");
}

TEST_F(ParserTest, ExpressionStatementUnaryDelete) {
  TEST_PARSER("delete foo;\n");
}

TEST_F(ParserTest, ExpressionStatementUnaryTypeOf) {
  TEST_PARSER("typeof foo;\n");
}

TEST_F(ParserTest, ExpressionStatementUnaryVoid) {
  TEST_PARSER("void foo;\n");
}

TEST_F(ParserTest, ExpressionStatementName) {
  TEST_PARSER("false;\n");
  TEST_PARSER("foo;\n");
  TEST_PARSER("null;\n");
  TEST_PARSER("true;\n");
}

TEST_F(ParserTest, ExpressionStatementYield) {
  TEST_PARSER("yield;\n");
  TEST_PARSER("yield 1;\n");
  TEST_PARSER("yield* foo();\n");
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

TEST_F(ParserTest, LabeledStatement) {
  // There are no label indentation for toplevel label.
  TEST_PARSER(
      "label:\n"
      "foo;\n");

  TEST_PARSER(
      "{\n"
      " label:\n"
      "  foo;\n"
      "}\n");
}

TEST_F(ParserTest, SwitchStatement) {
  TEST_PARSER(
      "switch (foo) {\n"
      "  case 1:\n"
      "    one;\n"
      "    break;\n"
      "  case 2:\n"
      "  case 3:\n"
      "    two_or_three;\n"
      "  default:\n"
      "    others;\n"
      "}\n");
}

TEST_F(ParserTest, ThrowStatement) {
  TEST_PARSER("throw foo;\n");
}

TEST_F(ParserTest, TryStatement) {
  TEST_PARSER(
      "try {\n"
      "  foo;\n"
      "} catch (bar) {\n"
      "  baz;\n"
      "}\n");

  TEST_PARSER(
      "try {\n"
      "  foo;\n"
      "} finally {\n"
      "  baz;\n"
      "}\n");

  TEST_PARSER(
      "try {\n"
      "  foo;\n"
      "} catch (bar) {\n"
      "  baz;\n"
      "} finally {\n"
      "  quux;\n"
      "}\n");
}

TEST_F(ParserTest, WhileStatement) {
  TEST_PARSER(
      "while (foo) {\n"
      "  bar;\n"
      "}\n");
}

}  // namespace internal
}  // namespace joana

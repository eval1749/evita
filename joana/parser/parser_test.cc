// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "joana/parser/parser.h"

#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/public/parser_options.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace parser {

//
// ParserTest
//
class ParserTest : public LexerTestBase {
 protected:
  ParserTest() = default;
  ~ParserTest() override = default;

  std::string Parse(base::StringPiece script_text,
                    const ParserOptions& options);
  std::string Parse(base::StringPiece script_text);

  const ast::Node& ParseBindingElement(base::StringPiece text);

  std::string ToString(const ast::Node& node,
                       const ast::Node* module = nullptr);

 private:
  DISALLOW_COPY_AND_ASSIGN(ParserTest);
};

std::string ParserTest::Parse(base::StringPiece script_text,
                              const ParserOptions& options) {
  PrepareSouceCode(script_text);
  Parser parser(&context(), source_code().range(), options);
  const auto& module = parser.Run();
  std::ostringstream ostream;
  ostream << AsPrintableTree(module) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string ParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

const ast::Node& ParserTest::ParseBindingElement(
    base::StringPiece script_text) {
  PrepareSouceCode(script_text);
  Parser parser(&context(), source_code().range(), {});
  parser.SkipCommentTokens();
  const auto& result = parser.ParseBindingElement();
  parser.Finish();
  return result;
}

std::string ParserTest::ToString(const ast::Node& node,
                                 const ast::Node* module) {
  std::ostringstream ostream;
  ostream << AsPrintableTree(node) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(ParserTest, Externs) {
  EXPECT_EQ(
      "Externs\n"
      "+--JsDocDocument\n"
      "|  +--JsDocText |/**|\n"
      "|  +--JsDocTag\n"
      "|  |  +--Name |@fileoverview|\n"
      "|  |  +--JsDocText ||\n"
      "|  +--JsDocTag\n"
      "|  |  +--Name |@externs|\n"
      "|  +--JsDocText |*/|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@const|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName\n"
      "|  |  |  |  |  +--Name |Object|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      Parse("/** @fileoverview @externs */\n"
            "/** @const {!Object} */\n"
            "var foo;\n"));
}

TEST_F(ParserTest, AnnotateExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--Annotation\n"
      "|  |  +--JsDocDocument\n"
      "|  |  |  +--JsDocText |/**|\n"
      "|  |  |  +--JsDocTag\n"
      "|  |  |  |  +--Name |@type|\n"
      "|  |  |  |  +--TypeName\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  |  +--GroupExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n",
      Parse("return /** @type {number} */(foo);"));
}

TEST_F(ParserTest, AnnotateExpressionError) {
  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--Annotation\n"
      "|  |  +--JsDocDocument\n"
      "|  |  |  +--JsDocText |/**|\n"
      "|  |  |  +--JsDocTag\n"
      "|  |  |  |  +--Name |@type|\n"
      "|  |  |  |  +--TypeName\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "PASER_ERROR_EXPRESSION_UNEXPECT_ANNOTATION@7:28\n",
      //     0123456789012345678901234567890123456789
      Parse("return /** @type {number} */ foo;"))
      << "only group expression has an annotation.";
}

TEST_F(ParserTest, AnnotateStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--LetStatement\n"
      "|  |  +--BindingNameElement\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      Parse("/** @type {number} */ let foo = 1;"));
}

TEST_F(ParserTest, AsyncFunction) {
  EXPECT_EQ(
      "Module\n"
      "+--Function<Async>\n"
      "|  +--Name |foo|\n"
      "|  +--ParameterList |()|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ReturnStatement\n"
      "|  |  |  +--NumericLiteral |1|\n",
      Parse("async function foo() { return 1; }"));
}

TEST_F(ParserTest, AsyncKeyword) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |async|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |async|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      Parse("async = async(1);"));
}

TEST_F(ParserTest, AutomaticSemicolon) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n",
      Parse("foo // comment"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n",
      Parse("foo /* comment */"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("foo // comment\nbar"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("foo /* comment */\nbar"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("foo /* comment\n */bar"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<++>\n"
      "|  |  +--Punctuator |++|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n",
      Parse("foo\n++bar"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<++>\n"
      "|  |  +--Punctuator |++|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "PASER_ERROR_STATEMENT_UNEXPECT_NEWLINE@4:6\n"
      "PASER_ERROR_EXPRESSION_UNEXPECT_NEWLINE@4:6\n"
      "PASER_ERROR_STATEMENT_EXPECT_SEMICOLON@0:3\n"
      "PASER_ERROR_STATEMENT_EXPECT_SEMICOLON@4:9\n",
      Parse("foo\n++bar", ParserOptions::Builder()
                              .set_disable_automatic_semicolon(true)
                              .Build()));
}

TEST_F(ParserTest, BlockStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--BlockStatement\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n",
      Parse("{\n"
            "  foo;\n"
            "  bar;\n"
            "}\n"));
}

TEST_F(ParserTest, Bracket) {
  EXPECT_EQ(
      "Module\n"
      "+--BlockStatement |{|\n"
      "PASER_ERROR_BRACKET_EXPECT_RBRACE@0:1\n",
      Parse("{"));
}

TEST_F(ParserTest, BreakStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--DoStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--BreakStatement\n"
      "|  |  |  +--Empty ||\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("do { break; } while (bar);"));

  EXPECT_EQ(
      "Module\n"
      "+--BreakStatement\n"
      "|  +--Name |foo|\n",
      Parse("break foo;"));

  EXPECT_EQ(
      "Module\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--BreakStatement\n"
      "|  |  +--Empty ||\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n"
      "|  +--BreakStatement\n"
      "|  |  +--Empty ||\n",
      Parse("if (foo) break\n"
            "if (bar) break\n"));
}

TEST_F(ParserTest, ClassStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--Class\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement\n"
      "|  |  |  |  +--ReturnStatement\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n"
      "|  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement\n"
      "|  |  |  |  +--ReturnStatement\n"
      "|  |  |  |  |  +--NumericLiteral |2|\n",
      Parse("class Foo {\n"
            "  foo() { return 1; }\n"
            "  foo() { return 2; }\n"
            "}"));
}

TEST_F(ParserTest, ConstStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ConstStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("const foo = 1;"));

  EXPECT_EQ(
      "Module\n"
      "+--ConstStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--NumericLiteral |2|\n",
      Parse("const foo = 1, bar = 2;"));
}

TEST_F(ParserTest, ContinueStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--DoStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ContinueStatement\n"
      "|  |  |  +--Empty ||\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("do {\n"
            "  continue;\n"
            "} while (bar);\n"));

  EXPECT_EQ(
      "Module\n"
      "+--ContinueStatement\n"
      "|  +--Name |foo|\n",
      Parse("continue foo;"));

  EXPECT_EQ(
      "Module\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--ContinueStatement\n"
      "|  |  +--Empty ||\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n"
      "|  +--ContinueStatement\n"
      "|  |  +--Empty ||\n",
      Parse("if (foo) continue\n"
            "if (bar) continue\n"));
}

TEST_F(ParserTest, DebuggerStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |debugger|\n",
      Parse("debugger"))
      << "automatic semicolon";
}

TEST_F(ParserTest, DoStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--DoStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n",
      Parse("do {\n"
            "  foo;\n"
            "} while (bar);\n"));
}

TEST_F(ParserTest, EmptyStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--EmptyStatement ||\n",
      Parse(";\n"));
}

TEST_F(ParserTest, ExpressionArrayLiteral) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrayInitializer |[]|\n",
      Parse("[];"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrayInitializer\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("[1];"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrayInitializer\n"
      "|  |  +--ElisionExpression ||\n",
      Parse("[,];"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrayInitializer\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--ElisionExpression ||\n",
      Parse("[,,];"));
}

TEST_F(ParserTest, ExpressionArrayLiteralSpread) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrayInitializer\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  |  +--NumericLiteral |2|\n"
      "|  |  +--UnaryExpression<...>\n"
      "|  |  |  +--Punctuator |...|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |x|\n",
      Parse("[1, 2, ...x];\n"));
}

TEST_F(ParserTest, ExpressionArrowFunction) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrowFunction\n"
      "|  |  +--ParameterList\n"
      "|  |  |  +--BindingNameElement\n"
      "|  |  |  |  +--Name |a|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "|  |  +--BlockStatement\n"
      "|  |  |  +--ExpressionStatement\n"
      "|  |  |  |  +--CallExpression\n"
      "|  |  |  |  |  +--MemberExpression\n"
      "|  |  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  |  +--Name |console|\n"
      "|  |  |  |  |  |  +--Name |log|\n"
      "|  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  +--Name |a|\n",
      Parse("(a) => {\n"
            "  console.log(a);\n"
            "};\n"));
}

TEST_F(ParserTest, ExpressionArrowFunction0) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrowFunction\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("() => 1;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrowFunction\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--BinaryExpression<*>\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--Punctuator |*|\n"
      "|  |  |  +--NumericLiteral |2|\n",
      Parse("x => x * 2;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ArrowFunction\n"
      "|  |  +--ParameterList\n"
      "|  |  |  +--BindingNameElement\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--BindingNameElement\n"
      "|  |  |  |  +--Name |y|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "|  |  +--BinaryExpression<*>\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--Punctuator |*|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |y|\n",
      Parse("(x, y) => x * y;"));
}

TEST_F(ParserTest, ExpressionAssignment1) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |100|\n",
      Parse("foo = 100;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--BinaryExpression<+>\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |bar|\n"
      "|  |  |  +--Punctuator |+|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n",
      Parse("foo = bar + baz;"));
}

TEST_F(ParserTest, ExpressionAssignmentMultiple) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--AssignmentExpression<=>\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |bar|\n"
      "|  |  |  +--Punctuator |=|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n",
      Parse("foo = bar = baz;"));
}

TEST_F(ParserTest, ExpressionAssignmentOperation) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<+=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |+=|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |baz|\n",
      Parse("foo += baz;"));
}

TEST_F(ParserTest, ExpressionBinary) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--BinaryExpression<+>\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  |  +--Punctuator |+|\n"
      "|  |  +--BinaryExpression<*>\n"
      "|  |  |  +--NumericLiteral |2|\n"
      "|  |  |  +--Punctuator |*|\n"
      "|  |  |  +--NumericLiteral |3|\n",
      Parse("1 + 2 * 3;"));
}

TEST_F(ParserTest, ExpressionBinary2) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--BinaryExpression<*>\n"
      "|  |  +--GroupExpression\n"
      "|  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  +--NumericLiteral |2|\n"
      "|  |  +--Punctuator |*|\n"
      "|  |  +--NumericLiteral |3|\n",
      Parse("(1 + 2) * 3;"));
}

TEST_F(ParserTest, ExpressionCall0) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("foo();"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("foo(1);"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--CommaExpression\n"
      "|  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--NumericLiteral |2|\n",
      Parse("foo(1, 2);"));
}

TEST_F(ParserTest, ExpressionClass) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |x|\n"
      "|  |  +--Class\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--ObjectInitializer |{}|\n",
      Parse("var x = class {};"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |x|\n"
      "|  |  +--Class\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--ObjectInitializer |{}|\n",
      Parse("var x = class Foo {};"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |x|\n"
      "|  |  +--Class\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |Bar|\n"
      "|  |  |  +--ObjectInitializer\n"
      "|  |  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  |  +--Name |constructor|\n"
      "|  |  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  |  +--BlockStatement |{}|\n",
      Parse("var x = class Foo extends Bar { constructor() {} };"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |x|\n"
      "|  |  +--Class\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--ObjectInitializer\n"
      "|  |  |  |  +--Method<Static,Normal>\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  |  +--BlockStatement |{}|\n",
      Parse("var x = class Foo { static foo() {} };"));
}

TEST_F(ParserTest, ExpressionClassWithStatic) {
  EXPECT_EQ(
      "Module\n"
      "+--Class\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Generator>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      Parse("class Foo { static *foo() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Async>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      Parse("class Foo { static async foo() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Getter>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      Parse("class Foo { static get foo() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Setter>\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      Parse("class Foo { static set foo() {} }"));
}

TEST_F(ParserTest, ExpressionCall3) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--CommaExpression\n"
      "|  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--NumericLiteral |2|\n"
      "|  |  |  +--NumericLiteral |3|\n",
      Parse("foo(1, 2, 3);"));
}

TEST_F(ParserTest, ExpressionComma) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CommaExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |baz|\n",
      Parse("foo, bar, baz;"));
}

TEST_F(ParserTest, ExpressionFunctionAsync) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--Function<Async>\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement\n"
      "|  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = async function() {\n"
            "  1;\n"
            "};\n"));
}

TEST_F(ParserTest, ExpressionFunctionGenerator) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--Function<Generator>\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ParameterList\n"
      "|  |  |  |  +--BindingNameElement\n"
      "|  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--BlockStatement\n"
      "|  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = function*(x) {\n"
            "  1;\n"
            "};\n"));
}

TEST_F(ParserTest, ExpressionFunctionNormal) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--Function<Normal>\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ParameterList\n"
      "|  |  |  |  +--BindingNameElement\n"
      "|  |  |  |  |  +--Name |a|\n"
      "|  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BindingNameElement\n"
      "|  |  |  |  |  +--Name |b|\n"
      "|  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--BlockStatement\n"
      "|  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = function(a, b) {\n"
            "  1;\n"
            "};\n"));
}

TEST_F(ParserTest, ExpressionConditional) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ConditionalExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |baz|\n",
      Parse("foo ? bar : baz;"));
}

TEST_F(ParserTest, ExpressionMember) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--MemberExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Name |bar|\n",
      Parse("foo.bar;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ComputedMemberExpression\n"
      "|  |  +--MemberExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--NumericLiteral |0|\n",
      Parse("foo.bar[0];"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--ComputedMemberExpression\n"
      "|  |  |  +--MemberExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--Name |bar|\n"
      "|  |  |  +--NumericLiteral |0|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("foo.bar[0](1);"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--CallExpression\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ComputedMemberExpression\n"
      "|  |  |  |  +--MemberExpression\n"
      "|  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  +--Name |bar|\n"
      "|  |  |  |  +--NumericLiteral |0|\n"
      "|  |  |  +--NumericLiteral |1|\n"
      "|  |  +--NumericLiteral |2|\n",
      Parse("foo.bar[0](1)(2);"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--MemberExpression\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--CallExpression\n"
      "|  |  |  |  +--ComputedMemberExpression\n"
      "|  |  |  |  |  +--MemberExpression\n"
      "|  |  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  |  +--Name |bar|\n"
      "|  |  |  |  |  +--NumericLiteral |0|\n"
      "|  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--NumericLiteral |2|\n"
      "|  |  +--Name |baz|\n",
      Parse("foo.bar[0](1)(2).baz;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--MemberExpression\n"
      "|  |  +--MemberExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--Name |baz|\n",
      Parse("foo.bar.baz"));
}

TEST_F(ParserTest, ExpressionMemberError) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "PASER_ERROR_EXPRESSION_EXPECT_NAME@4:4\n",
      Parse("foo."))
      << "No member name after '.'";
}

TEST_F(ParserTest, ExpressionNew) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NewExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |Foo|\n",
      Parse("var foo = new Foo\n"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NewExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      Parse("var foo = new Foo(1);"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NewExpression\n"
      "|  |  |  +--MemberExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |testing|\n"
      "|  |  |  |  +--Name |Test|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      Parse("var foo = new testing.Test(1);"));

  EXPECT_EQ(
      "Module\n"
      "+--IfStatement\n"
      "|  +--MemberExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |new|\n"
      "|  |  +--Name |target|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |ok|\n",
      Parse("if (new.target)\n  ok;"));
}

TEST_F(ParserTest, ExpressionObjectLiteral) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--DelimiterExpression |,|\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--Name |bar|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--Method<NonStatic,Generator>\n"
      "|  |  |  |  +--Name |baz|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |2|\n"
      "|  |  |  +--Method<NonStatic,Async>\n"
      "|  |  |  |  +--Name |quux|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |3|\n"
      "|  |  |  +--Method<NonStatic,Getter>\n"
      "|  |  |  |  +--Name |getter|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |4|\n"
      "|  |  |  +--Method<NonStatic,Setter>\n"
      "|  |  |  |  +--Name |setter|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |5|\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--NumericLiteral |123|\n"
      "|  |  |  |  +--NumericLiteral |456|\n"
      "|  |  |  +--DelimiterExpression |,|\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--StringLiteral |'foo'|\n"
      "|  |  |  |  +--NumericLiteral |789|\n"
      "|  |  |  +--DelimiterExpression |,|\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--ArrayInitializer\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  |  +--NumericLiteral |6|\n"
      "|  |  |  +--DelimiterExpression |,|\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--ArrayInitializer\n"
      "|  |  |  |  |  +--NumericLiteral |2|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |7|\n",
      Parse("x = {\n"
            "  foo: 1,\n"
            "  bar() { 1; }\n"
            "  *baz() { 2; }\n"
            "  async quux() { 3; }\n"
            "  get getter() { 4; }\n"
            "  set setter() { 5; }\n"
            "  123: 456,\n"
            "  'foo': 789,\n"
            "  [1]: 6,\n"
            "  [2]() { 7; }\n"
            "};\n"));
}

TEST_F(ParserTest, ExpressionObjectLiteralWithArrowFunction) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--ArrowFunction\n"
      "|  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  +--DelimiterExpression |,|\n",
      Parse("x = { foo: x => x + 1, };"));
}

TEST_F(ParserTest, ExpressionObjectLiteralWithKeywords) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |aync|\n"
      "|  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { aync: 1 };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |get|\n"
      "|  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { get: 1 };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |set|\n"
      "|  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { set: 1 };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |static|\n"
      "|  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { static: 1 };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--Name |aync|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      Parse("x = { aync() {} };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--Name |get|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      Parse("x = { get() {} };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--Name |set|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      Parse("x = { set() {} };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Normal>\n"
      "|  |  |  |  +--Name |static|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      Parse("x = { static() {} };"));
}

TEST_F(ParserTest, ExpressionObjectLiteralAndPropertyName) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Getter>\n"
      "|  |  |  |  +--NumericLiteral |5e0|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { get 5e0() { 1; } };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Getter>\n"
      "|  |  |  |  +--StringLiteral |'foo'|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { get 'foo'() { 1; } };"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Method<NonStatic,Getter>\n"
      "|  |  |  |  +--ArrayInitializer\n"
      "|  |  |  |  |  +--NumericLiteral |1|\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement\n"
      "|  |  |  |  |  +--ExpressionStatement\n"
      "|  |  |  |  |  |  +--NumericLiteral |1|\n",
      Parse("x = { get [1]() { 1; } };"));
}

TEST_F(ParserTest, ExpressionUnary) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<!>\n"
      "|  |  +--Punctuator |!|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("!foo;"));
}

TEST_F(ParserTest, ExpressionUnaryAwait) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<await>\n"
      "|  |  +--Name |await|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("await foo;"));
}

TEST_F(ParserTest, ExpressionUnaryDelete) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<delete>\n"
      "|  |  +--Name |delete|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("delete foo;"));
}

TEST_F(ParserTest, ExpressionUnaryTypeOf) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<typeof>\n"
      "|  |  +--Name |typeof|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("typeof foo;"));
}

TEST_F(ParserTest, ExpressionUnaryVoid) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<void>\n"
      "|  |  +--Name |void|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("void foo;"));
}

TEST_F(ParserTest, ExpressionName) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--BooleanLiteral |false|\n",
      Parse("false;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n",
      Parse("foo;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--NullLiteral |null|\n",
      Parse("null;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--BooleanLiteral |true|\n",
      Parse("true;"));
}

TEST_F(ParserTest, ExpressionRegExp) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |re|\n"
      "|  |  +--RegExpLiteralExpression\n"
      "|  |  |  +--SequenceRegExp\n"
      "|  |  |  |  +--LiteralRegExp |a|\n"
      "|  |  |  |  +--RepeatRegExp\n"
      "|  |  |  |  |  +--LiteralRegExp |b|\n"
      "|  |  |  |  |  +--RegExpRepeat<*> |*|\n"
      "|  |  |  |  +--LiteralRegExp |c|\n"
      "|  |  |  +--Empty ||\n",
      Parse("var re = /ab*c/;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--ObjectInitializer\n"
      "|  |  |  +--Property\n"
      "|  |  |  |  +--Name |re|\n"
      "|  |  |  |  +--RegExpLiteralExpression\n"
      "|  |  |  |  |  +--SequenceRegExp\n"
      "|  |  |  |  |  |  +--AssertionRegExp |^|\n"
      "|  |  |  |  |  |  +--CaptureRegExp\n"
      "|  |  |  |  |  |  |  +--RepeatRegExp\n"
      "|  |  |  |  |  |  |  |  +--AnyCharRegExp |.|\n"
      "|  |  |  |  |  |  |  |  +--RegExpRepeat<+> |+|\n"
      "|  |  |  |  |  |  +--AssertionRegExp |$|\n"
      "|  |  |  |  |  +--Empty ||\n",
      Parse("foo = { re: /^(.+)$/ }"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |re|\n"
      "|  |  +--RegExpLiteralExpression\n"
      "|  |  |  +--LiteralRegExp |a|\n"
      "|  |  |  +--Empty ||\n"
      "+--ExpressionStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n",
      Parse("var re = /a/\nfoo\n"))
      << "'foo' is not regexp flags.";

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |re|\n"
      "|  |  +--RegExpLiteralExpression\n"
      "|  |  |  +--LiteralRegExp |bar|\n"
      "|  |  |  +--Name |u|\n",
      Parse("var re = /bar/u;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |re|\n"
      "|  |  +--RegExpLiteralExpression\n"
      "|  |  |  +--LiteralRegExp |=|\n"
      "|  |  |  +--Empty ||\n",
      Parse("var re = /=/;\n"))
      << "'/=' is not assignment operator";
}

TEST_F(ParserTest, ExpressionYield) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<yield>\n"
      "|  |  +--Name |yield|\n"
      "|  |  +--ElisionExpression ||\n",
      Parse("yield;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<yield>\n"
      "|  |  +--Name |yield|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("yield 1;"));

  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--UnaryExpression<yield_star>\n"
      "|  |  +--Name |yield*|\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n",
      Parse("yield* foo();"));
}

TEST_F(ParserTest, ForStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ForStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |index|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |0|\n"
      "|  +--BinaryExpression<<>\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |index|\n"
      "|  |  +--Punctuator |<|\n"
      "|  |  +--NumericLiteral |10|\n"
      "|  +--UnaryExpression<++>\n"
      "|  |  +--Punctuator |++|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |index|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |call|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |index|\n",
      Parse("for (index = 0; index < 10; ++index)\n"
            "  call(index);\n"));
}

TEST_F(ParserTest, ForStatementInfinite) {
  EXPECT_EQ(
      "Module\n"
      "+--ForStatement\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |call|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |index|\n",
      Parse("for (;;)\n"
            "  call(index);\n"));
}

TEST_F(ParserTest, ForInStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ForInStatement\n"
      "|  +--BinaryExpression<in>\n"
      "|  |  +--MemberExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  +--Name |x|\n"
      "|  |  +--Name |in|\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |call|\n"
      "|  |  |  +--MemberExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--Name |x|\n",
      Parse("for (foo.x in bar)\n"
            "  call(foo.x);\n"));
}

TEST_F(ParserTest, ForOfStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ForOfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |element|\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |elements|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--CallExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |call|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |element|\n",
      Parse("for (const element of elements)\n"
            "  call(element);\n"));
}

TEST_F(ParserTest, FunctionStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--Function<Normal>\n"
      "|  +--Name |foo|\n"
      "|  +--ParameterList\n"
      "|  |  +--BindingNameElement\n"
      "|  |  |  +--Name |a|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement\n"
      "|  |  |  +--Name |b|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement\n"
      "|  |  |  +--Name |c|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--CallExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |bar|\n"
      "|  |  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  +--Name |a|\n"
      "|  |  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  |  +--Name |b|\n",
      Parse("function foo(a, b, c) {\n"
            "  bar(a + b);\n"
            "}\n"));
}

TEST_F(ParserTest, IfStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n",
      Parse("if (foo)\n"
            "  bar;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--IfElseStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |baz|\n",
      Parse("if (foo)\n"
            "  bar;\n"
            "else\n"
            "  baz;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo2|\n"
      "+--IfStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |bar|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar2|\n",
      Parse("if (foo) foo2\n"
            "if (bar) bar2\n"));
}

TEST_F(ParserTest, LabeledStatement) {
  // There are no label indentation for toplevel label.
  EXPECT_EQ(
      "Module\n"
      "+--LabeledStatement\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |foo|\n",
      Parse("label:\n"
            "foo;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--BlockStatement\n"
      "|  +--LabeledStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n",
      Parse("{\n"
            " label:\n"
            "  foo;\n"
            "}\n"));

  EXPECT_EQ(
      "Module\n"
      "+--SwitchStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--LabeledStatement\n"
      "|  |  +--EmptyStatement ||\n",
      Parse("switch (foo) {\n"
            "  default:\n"
            "}\n"))
      << "Insert semicolon for 'default' label";
}

TEST_F(ParserTest, LetStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("let foo = 1;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n",
      Parse("let foo = 1, bar;\n"));
}

TEST_F(ParserTest, ReturnStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--ElisionExpression ||\n",
      Parse("return;"))
      << "We have elision expression for return.";

  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--NumericLiteral |1|\n",
      Parse("return 1;"));

  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--CommaExpression\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  |  +--NumericLiteral |2|\n",
      Parse("return 1, 2;"));
}

TEST_F(ParserTest, SwitchStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--SwitchStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--CaseClause\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |one|\n"
      "|  +--BreakStatement\n"
      "|  |  +--Empty ||\n"
      "|  +--CaseClause\n"
      "|  |  +--NumericLiteral |2|\n"
      "|  |  +--CaseClause\n"
      "|  |  |  +--NumericLiteral |3|\n"
      "|  |  |  +--ExpressionStatement\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |two_or_three|\n"
      "|  +--LabeledStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |others|\n",
      Parse("switch (foo) {\n"
            "  case 1:\n"
            "    one;\n"
            "    break;\n"
            "  case 2:\n"
            "  case 3:\n"
            "    two_or_three;\n"
            "  default:\n"
            "    others;\n"
            "}\n"));

  EXPECT_EQ(
      "Module\n"
      "+--SwitchStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--CaseClause\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  |  +--EmptyStatement ||\n",
      Parse("switch (foo) { case 1: }"))
      << "Insert empty statement";
}

TEST_F(ParserTest, ThrowStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--ThrowStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n",
      Parse("throw foo;"));
}

TEST_F(ParserTest, TryStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--TryCatchStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  +--GroupExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n",
      Parse("try {\n"
            "  foo;\n"
            "} catch (bar) {\n"
            "  baz;\n"
            "}\n"));

  EXPECT_EQ(
      "Module\n"
      "+--TryFinallyStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n",
      Parse("try {\n"
            "  foo;\n"
            "} finally {\n"
            "  baz;\n"
            "}\n"));

  EXPECT_EQ(
      "Module\n"
      "+--TryCatchFinallyStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  +--GroupExpression\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |quux|\n",
      Parse("try {\n"
            "  foo;\n"
            "} catch (bar) {\n"
            "  baz;\n"
            "} finally {\n"
            "  quux;\n"
            "}\n"));
}

TEST_F(ParserTest, VarStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n",
      Parse("var foo = 1;\n"));

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n",
      Parse("var foo = 1, bar;\n"));
}

TEST_F(ParserTest, WhileStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--WhileStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |bar|\n",
      Parse("while (foo) {\n"
            "  bar;\n"
            "}\n"));
}

TEST_F(ParserTest, WithStatement) {
  EXPECT_EQ(
      "Module\n"
      "+--WithStatement\n"
      "|  +--ReferenceExpression\n"
      "|  |  +--Name |foo|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ReferenceExpression\n"
      "|  |  |  +--Name |bar|\n",
      Parse("with (foo)\n"
            "  bar;\n"));
}

// Bindings
TEST_F(ParserTest, ParseArrayBindingPattern) {
  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n",
      ToString(ParseBindingElement("[]")))
      << "no elements";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo]")))
      << "one element";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo, bar]")))
      << "two elements";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ArrayInitializer\n"
      "|  +--NumericLiteral |1|\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo, bar] = [1]")))
      << "two elements with initializer";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n",
      ToString(ParseBindingElement("[foo,]")))
      << "no element after comma";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingCommaElement |,|\n",
      ToString(ParseBindingElement("[foo,,]")))
      << "no element after commas";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo,,bar]")))
      << "no element between commas";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |baz|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo, bar, baz]")))
      << "three elements";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingRestElement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[foo, ...bar]")))
      << "rest element";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--ArrayBindingPattern\n"
      "|  +--ElisionExpression ||\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[[foo]]")))
      << "array in array";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--ObjectBindingPattern\n"
      "|  +--ElisionExpression ||\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("[{foo}]")))
      << "object in array";
}

TEST_F(ParserTest, ParseArrayBindingPatternError) {
  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "PASER_ERROR_BRACKET_EXPECT_RBRACKET@0:1\n",
      ToString(ParseBindingElement("[")))
      << "no closing bracket";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingRestElement\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |baz|\n"
      "|  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_UNEXPECT_REST@6:12\n",
      ToString(ParseBindingElement("[foo, ...bar, baz]")))
      << "rest element should be the last element";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--ArrayBindingPattern\n"
      "|  +--ElisionExpression ||\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_EXPECT_COMMA@1:5\n"
      "PASER_ERROR_BRACKET_UNEXPECT_RBRACE@5:11\n"
      "PASER_ERROR_BRACKET_EXPECT_RBRACKET@5:11\n",
      ToString(ParseBindingElement("{foo [bar }")))
      << "Missing right bracket";

  EXPECT_EQ(
      "ArrayBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_INVALID_ELEMENT@6:7\n",
      ToString(ParseBindingElement("[foo, = 1, bar]")))
      << "no name before equal";
}

TEST_F(ParserTest, ParseBindingNameElement) {
  EXPECT_EQ(
      "BindingNameElement\n"
      "+--Name |foo|\n"
      "+--ElisionExpression ||\n",
      ToString(ParseBindingElement("foo")))
      << "without initializer";

  EXPECT_EQ(
      "BindingNameElement\n"
      "+--Name |foo|\n"
      "+--NumericLiteral |1|\n",
      ToString(ParseBindingElement("foo = 1")))
      << "with initializer";
}

TEST_F(ParserTest, ParseObjectBindingPattern) {
  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n",
      ToString(ParseBindingElement("{}")))
      << "no elements";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo}")))
      << "one element";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--NumericLiteral |1|\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo} = 1")))
      << "with initializer";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--NumericLiteral |1|\n",
      ToString(ParseBindingElement("{foo = 1}")))
      << "one element with initializer";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingProperty\n"
      "|  +--Name |foo|\n"
      "|  +--BindingNameElement\n"
      "|  |  +--Name |bar|\n"
      "|  |  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo: bar}")))
      << "property";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingProperty\n"
      "|  +--Name |foo|\n"
      "|  +--ObjectBindingPattern\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--BindingProperty\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ArrayBindingPattern\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BindingNameElement\n"
      "|  |  |  |  |  +--Name |baz|\n"
      "|  |  |  |  |  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo: {bar: [baz]} }")))
      << "nested property";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo, bar}")))
      << "two elements";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n",
      ToString(ParseBindingElement("{foo, bar,}")))
      << "two elements with trailing comma";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |baz|\n"
      "|  +--ElisionExpression ||\n",
      ToString(ParseBindingElement("{foo, bar, baz}")))
      << "three elements";
}

TEST_F(ParserTest, ParseObjectBindingPatternError) {
  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "PASER_ERROR_BINDING_UNEXPECT_COMMA@1:2\n",
      ToString(ParseBindingElement("{,}")))
      << "only comma";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_UNEXPECT_COMMA@1:2\n",
      ToString(ParseBindingElement("{, foo}")))
      << "no name before comma";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_EXPECT_COMMA@1:5\n",
      ToString(ParseBindingElement("{foo bar}")))
      << "no comma between names";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--Invalid |}|\n"
      "PASER_ERROR_EXPRESSION_INVALID@7:8\n",
      ToString(ParseBindingElement("{foo = }")))
      << "no expression after equal";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingProperty\n"
      "|  +--Name |foo|\n"
      "|  +--BindingInvalidElement |}|\n"
      "PASER_ERROR_BINDING_INVALID_ELEMENT@6:7\n",
      ToString(ParseBindingElement("{foo: }")))
      << "no element after colon";

  EXPECT_EQ(
      "ObjectBindingPattern\n"
      "+--ElisionExpression ||\n"
      "+--BindingNameElement\n"
      "|  +--Name |foo|\n"
      "|  +--ElisionExpression ||\n"
      "+--BindingCommaElement |,|\n"
      "+--BindingNameElement\n"
      "|  +--Name |bar|\n"
      "|  +--ElisionExpression ||\n"
      "PASER_ERROR_BINDING_INVALID_ELEMENT@6:7\n",
      ToString(ParseBindingElement("{foo, = 1, bar}")))
      << "no name before equal";
}

}  // namespace parser
}  // namespace joana

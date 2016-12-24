// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "joana/parser/parser.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/module.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parse.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_context_builder.h"
#include "joana/parser/simple_error_sink.h"
#include "joana/parser/simple_formatter.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace parser {

class ParserTest : public ::testing::Test {
 protected:
  ParserTest() = default;
  ~ParserTest() override = default;

  std::string Parse(base::StringPiece script_text,
                    const ParserOptions& options);
  std::string Parse(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(ParserTest);
};

std::string ParserTest::Parse(base::StringPiece script_text,
                              const ParserOptions& options) {
  Zone zone("ParserTest");
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
  Parser parser(context.get(), source_code.range(), options);
  const auto& module = parser.Run().As<ast::Module>();
  SimpleFormatter(&ostream).Format(module);
  for (const auto& error : error_sink.errors())
    ostream << error << std::endl;
  for (const auto& statement : module.statements()) {
    auto* js_doc = module.JsDocFor(statement);
    if (!js_doc)
      continue;
    ostream << statement << ':' << *js_doc << std::endl;
  }
  return ostream.str();
}

std::string ParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

#define TEST_PARSER(script_text)           \
  {                                        \
    auto* const source = script_text;      \
    EXPECT_EQ(source, Parse(script_text)); \
  }

TEST_F(ParserTest, JsDoc) {
  EXPECT_EQ(
      "let foo = 1;\n"
      "LetStatement([22-34], \"let foo = 1;\"):"
      "JsDoc([0-21], \"/** @type {number} */\")\n",
      Parse("/** @type {number} */ let foo = 1;"));
}

TEST_F(ParserTest, AsyncFunction) {
  TEST_PARSER(
      "async function foo() {\n"
      "  return 1;\n"
      "}\n");
}

TEST_F(ParserTest, AsyncKeyword) {
  TEST_PARSER("async = async(1);\n");
}

TEST_F(ParserTest, AutomaticSemicolon) {
  EXPECT_EQ("foo;\n", Parse("foo // comment"));
  EXPECT_EQ("foo;\n", Parse("foo /* comment */"));
  EXPECT_EQ(
      "foo;\n"
      "bar;\n",
      Parse("foo // comment\nbar"));
  EXPECT_EQ(
      "foo;\n"
      "bar;\n",
      Parse("foo /* comment */\nbar"));
  EXPECT_EQ(
      "foo;\n"
      "bar;\n",
      Parse("foo /* comment\n */bar"));
  EXPECT_EQ(
      "foo;\n"
      "++bar;\n",
      Parse("foo\n++bar"));
  EXPECT_EQ(
      "foo;\n"
      "++bar;\n"
      "PASER_ERROR_STATEMENT_UNEXPECT_NEWLINE@4:6\n"
      "PASER_ERROR_EXPRESSION_UNEXPECT_NEWLINE@4:6\n"
      "PASER_ERROR_STATEMENT_EXPECT_SEMICOLON@0:3\n"
      "PASER_ERROR_STATEMENT_EXPECT_SEMICOLON@4:9\n",
      Parse("foo\n++bar",
            ParserOptionsBuilder().SetAutomaticSemicolon(false).Build()));
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
  TEST_PARSER("break foo;\n");
  EXPECT_EQ(
      "if (foo)\n"
      "  break;\n"
      "if (bar)\n"
      "  break;\n",
      Parse("if (foo) break\n"
            "if (bar) break\n"));
}

TEST_F(ParserTest, ClassStatement) {
  TEST_PARSER(
      "class Foo {\n"
      "  foo() { return 1; }\n"
      "  foo() { return 2; }\n"
      "}\n");
}

TEST_F(ParserTest, ConstStatement) {
  TEST_PARSER("const foo = 1;\n");
  TEST_PARSER("const foo = 1, bar = 2;\n");
}

TEST_F(ParserTest, ContinueStatement) {
  TEST_PARSER(
      "do {\n"
      "  continue;\n"
      "} while (bar);\n");
  TEST_PARSER("continue foo;\n");
  EXPECT_EQ(
      "if (foo)\n"
      "  continue;\n"
      "if (bar)\n"
      "  continue;\n",
      Parse("if (foo) continue\n"
            "if (bar) continue\n"));
}

TEST_F(ParserTest, DebuggerStatement) {
  EXPECT_EQ("debugger;\n", Parse("debugger")) << "automatic semicolon";
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

TEST_F(ParserTest, ExpressionArrayLiteral) {
  TEST_PARSER("[];\n");
  TEST_PARSER("[1];\n");
  TEST_PARSER("[,];\n");
  TEST_PARSER("[,,];\n");
}

TEST_F(ParserTest, ExpressionArrayLiteralSpread) {
  TEST_PARSER("[1, 2, ...x];\n");
}

TEST_F(ParserTest, ExpressionArrowFunction) {
  TEST_PARSER(
      "(a) => {\n"
      "  console.log(a);\n"
      "};\n");
}

TEST_F(ParserTest, ExpressionArrowFunction0) {
  TEST_PARSER("() => 1;\n");
}

TEST_F(ParserTest, ExpressionArrowFunction1) {
  TEST_PARSER("x => x * 2;\n");
}

TEST_F(ParserTest, ExpressionArrowFunction2) {
  TEST_PARSER("(x, y) => x * y;\n");
}

TEST_F(ParserTest, ExpressionAssignment1) {
  TEST_PARSER("foo = 100;\n");
  TEST_PARSER("foo = bar + baz;\n");
}

TEST_F(ParserTest, ExpressionAssignmentMultiple) {
  TEST_PARSER("foo = bar = baz;\n");
}

TEST_F(ParserTest, ExpressionAssignmentOperation) {
  TEST_PARSER("foo += baz;\n");
}

TEST_F(ParserTest, ExpressionBinary) {
  TEST_PARSER("1 + 2 * 3;\n");
}

TEST_F(ParserTest, ExpressionBinary2) {
  TEST_PARSER("(1 + 2) * 3;\n");
}

TEST_F(ParserTest, ExpressionCall0) {
  TEST_PARSER("foo();\n");
}

TEST_F(ParserTest, ExpressionCall1) {
  TEST_PARSER("foo(1);\n");
}

TEST_F(ParserTest, ExpressionCall2) {
  TEST_PARSER("foo(1, 2);\n");
}

TEST_F(ParserTest, ExpressionClass) {
  TEST_PARSER("var x = class {};\n");
  TEST_PARSER("var x = class Foo {};\n");
  TEST_PARSER("var x = class Foo extends Bar { constructor() {} };\n");
  TEST_PARSER("var x = class Foo { static foo() {} };\n");
}

TEST_F(ParserTest, ExpressionClassWithStatic) {
  TEST_PARSER("class Foo { static *foo() {} }\n");
  TEST_PARSER("class Foo { static async foo() {} }\n");
  TEST_PARSER("class Foo { static get foo() {} }\n");
  TEST_PARSER("class Foo { static set foo() {} }\n");
}

TEST_F(ParserTest, ExpressionCall3) {
  TEST_PARSER("foo(1, 2, 3);\n");
}

TEST_F(ParserTest, ExpressionComma) {
  TEST_PARSER("foo, bar, baz;\n");
}

TEST_F(ParserTest, ExpressionFunctionAsync) {
  TEST_PARSER(
      "x = async function() {\n"
      "  1;\n"
      "};\n");
}

TEST_F(ParserTest, ExpressionFunctionGenerator) {
  TEST_PARSER(
      "x = function*(x) {\n"
      "  1;\n"
      "};\n");
}

TEST_F(ParserTest, ExpressionFunctionNormal) {
  TEST_PARSER(
      "x = function(a, b) {\n"
      "  1;\n"
      "};\n");
}

TEST_F(ParserTest, ExpressionConditional) {
  TEST_PARSER("foo ? bar : baz;\n");
}

TEST_F(ParserTest, ExpressionMember) {
  TEST_PARSER("foo.bar;\n");
  TEST_PARSER("foo.bar[0];\n");
  TEST_PARSER("foo.bar[0](1);\n");
  TEST_PARSER("foo.bar[0](1)(2);\n");
  TEST_PARSER("foo.bar[0](1)(2).baz;\n");

  EXPECT_EQ("foo;\nPASER_ERROR_EXPRESSION_EXPECT_NAME@4:4\n", Parse("foo."));
}

TEST_F(ParserTest, ExpressionNew) {
  EXPECT_EQ("var foo = new Foo();\n", Parse("var foo = new Foo\n"));
  TEST_PARSER("var foo = new Foo(1);\n");
  TEST_PARSER("var foo = new testing.Test(1);\n");
  TEST_PARSER("if (new.target)\n  ok;\n");
}

TEST_F(ParserTest, ExpressionObjectLiteral) {
  TEST_PARSER(
      "x = {\n"
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
      "};\n");
}

TEST_F(ParserTest, ExpressionObjectLiteralWithArrowFunction) {
  TEST_PARSER("x = { foo: x => x + 1, };\n");
}

TEST_F(ParserTest, ExpressionObjectLiteralWithKeywords) {
  TEST_PARSER("x = { aync: 1 };\n");
  TEST_PARSER("x = { get: 1 };\n");
  TEST_PARSER("x = { set: 1 };\n");
  TEST_PARSER("x = { static: 1 };\n");

  TEST_PARSER("x = { aync() {} };\n");
  TEST_PARSER("x = { get() {} };\n");
  TEST_PARSER("x = { set() {} };\n");
  TEST_PARSER("x = { static() {} };\n");
}

TEST_F(ParserTest, ExpressionObjectLiteralAndPropertyName) {
  TEST_PARSER("x = { get 5e0() { 1; } };\n");
  TEST_PARSER("x = { get 'foo'() { 1; } };\n");
  TEST_PARSER("x = { get [1]() { 1; } };\n");
}

TEST_F(ParserTest, ExpressionUnary) {
  TEST_PARSER("!foo;\n");
}

TEST_F(ParserTest, ExpressionUnaryAwait) {
  TEST_PARSER("await foo;\n");
}

TEST_F(ParserTest, ExpressionUnaryDelete) {
  TEST_PARSER("delete foo;\n");
}

TEST_F(ParserTest, ExpressionUnaryTypeOf) {
  TEST_PARSER("typeof foo;\n");
}

TEST_F(ParserTest, ExpressionUnaryVoid) {
  TEST_PARSER("void foo;\n");
}

TEST_F(ParserTest, ExpressionName) {
  TEST_PARSER("false;\n");
  TEST_PARSER("foo;\n");
  TEST_PARSER("null;\n");
  TEST_PARSER("true;\n");
}

TEST_F(ParserTest, ExpressionRegExp) {
  TEST_PARSER("var re = /ab*c/;\n");
  TEST_PARSER("var re = /bar/u;\n");
  TEST_PARSER("var re = { re: /^(.+)$/ };\n");
  TEST_PARSER("var re = /=/;\n");  // "'/=' is not assignment operator";
  TEST_PARSER("var re = /a{2}/;\n");
  TEST_PARSER("var re = /a{2,}/;\n");
  EXPECT_EQ("var re = /a/;\nfoo;\n", Parse("var re = /a/\nfoo\n"))
      << "'foo' is not regexp flags.";
  EXPECT_EQ("/(?:)/;\n", Parse("/(?:)/")) << "empty regexp";
  // non-strict regexp
  TEST_PARSER("/(foo|)/;\n");  // '|' is non-syntax char
  TEST_PARSER("/()foo/;\n");   // '()' is non-syntax char
  TEST_PARSER("/#{foo}/;\n");  // '{}' is non-syntax char
}

TEST_F(ParserTest, ExpressionYield) {
  TEST_PARSER("yield;\n");
  TEST_PARSER("yield 1;\n");
  TEST_PARSER("yield* foo();\n");
}

TEST_F(ParserTest, ForStatement) {
  TEST_PARSER(
      "for (index = 0; index < 10; ++index)\n"
      "  call(index);\n");
}

TEST_F(ParserTest, ForStatementInfinite) {
  TEST_PARSER(
      "for (;;)\n"
      "  call(index);\n");
}

TEST_F(ParserTest, ForInStatement) {
  TEST_PARSER(
      "for (foo.x in bar)\n"
      "  call(foo.x);\n");
}

TEST_F(ParserTest, ForOfStatement) {
  TEST_PARSER(
      "for (const element of elements)\n"
      "  call(element);\n");
}

TEST_F(ParserTest, FunctionStatement) {
  TEST_PARSER(
      "function foo(a, b, c) {\n"
      "  bar(a + b);\n"
      "}\n");
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
  EXPECT_EQ(
      "if (foo)\n"
      "  foo2;\n"
      "if (bar)\n"
      "  bar2;\n",
      Parse("if (foo) foo2\n"
            "if (bar) bar2\n"));
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

  EXPECT_EQ(
      "switch (foo) {\n"
      "  default:\n"
      "    ;\n"
      "}\n",
      Parse("switch (foo) {\n"
            "  default:\n"
            "}\n"))
      << "Insert semicolon for 'default' label";
}

TEST_F(ParserTest, LetStatement) {
  TEST_PARSER("let foo = 1;\n");
  TEST_PARSER("let foo = 1, bar;\n");
}

TEST_F(ParserTest, ReturnStatement) {
  TEST_PARSER("return;\n");
  TEST_PARSER("return 1;\n");
  TEST_PARSER("return 1, 2;\n");
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
  EXPECT_EQ(
      "switch (foo) {\n"
      "  case 1:\n"
      "    ;\n"
      "}\n",
      Parse("switch (foo) { case 1: }"))
      << "Insert empty statement";
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

TEST_F(ParserTest, VarStatement) {
  TEST_PARSER("var foo = 1;\n");
  TEST_PARSER("var foo = 1, bar;\n");
}

TEST_F(ParserTest, WhileStatement) {
  TEST_PARSER(
      "while (foo) {\n"
      "  bar;\n"
      "}\n");
}

TEST_F(ParserTest, WithStatement) {
  TEST_PARSER(
      "with (foo)\n"
      "  bar;\n");
}

}  // namespace parser
}  // namespace joana

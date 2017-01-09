// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/analyzer/environment_builder.h"

#include "joana/analyzer/analyzer_test_base.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/print_as_tree.h"
#include "joana/analyzer/type.h"
#include "joana/analyzer/value.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"
#include "joana/base/escaped_string_piece.h"
#include "joana/parser/public/parse.h"
#include "joana/testing/simple_error_sink.h"

namespace joana {
namespace analyzer {

//
// EnvironmentBuilderTest
//
class EnvironmentBuilderTest : public AnalyzerTestBase {
 protected:
  EnvironmentBuilderTest() = default;
  ~EnvironmentBuilderTest() override = default;

  std::string RunOn(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilderTest);
};

std::string EnvironmentBuilderTest::RunOn(base::StringPiece script_text) {
  const auto& module = ParseAsModule(script_text);
  const auto& context = NewContext();
  EnvironmentBuilder builder(context.get());
  builder.RunOn(module);
  std::ostringstream ostream;
  ostream << AsPrintableTree(*context, module) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(EnvironmentBuilderTest, Class) {
  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> $Function@1002\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> $Function@1003\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { bar() {} baz() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Annotation\n"
      "|  |  |  +--JsDocDocument\n"
      "|  |  |  |  +--JsDocText |/**|\n"
      "|  |  |  |  +--JsDocTag\n"
      "|  |  |  |  |  +--Name |@param|\n"
      "|  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  |  +--ReferenceExpression $Variable@1003\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  +--JsDocText |*/|\n"
      "|  |  |  +--Method<NonStatic,Normal> $Function@1002\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--ParameterList\n"
      "|  |  |  |  |  +--BindingNameElement $Variable@1003\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { /** @param {number} x */ foo(x) {} }"));
}

TEST_F(EnvironmentBuilderTest, ClassAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement $Variable@1002\n"
      "|  |  +--Name |a|\n"
      "|  |  +--Class $Class@1001 {class@1001}\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--ObjectInitializer |{}|\n",
      RunOn("var a = class {};"));
}

TEST_F(EnvironmentBuilderTest, ClassError) {
  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--NumericLiteral |1|\n"
      "ANALYZER_ERROR_ENVIRONMENT_EXPECT_METHOD@12:13\n",
      RunOn("class Foo { 1 }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> $Function@1002\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> $Function@1003\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@12:29\n",
      RunOn("class Foo { bar() {} bar() {} }"));
}

TEST_F(EnvironmentBuilderTest, ClassErrorConstructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Async> $Function@1002\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:34\n",
      RunOn("class Foo { async constructor() {} }"))
      << "constructor can not be async.";

  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Generator> $Function@1002\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:29\n",
      RunOn("class Foo { *constructor() {} }"))
      << "constructor can not be generator.";

  EXPECT_EQ(
      "Module\n"
      "+--Class $Class@1001 {class Foo@1001}\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Normal> $Function@1002\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:35\n",
      RunOn("class Foo { static constructor() {} }"))
      << "constructor can not be static.";
}

TEST_F(EnvironmentBuilderTest, ComputedMemberExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> $Variable@1002\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@const|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ComputedMemberExpression $Property@1005\n"
      "|  |  |  +--ReferenceExpression $Variable@1002\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--MemberExpression $Property@1004\n"
      "|  |  |  |  +--ReferenceExpression $Variable@1003\n"
      "|  |  |  |  |  +--Name |Symbol|\n"
      "|  |  |  |  +--Name |iterator|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo[Symbol.iterator]\n"));
}

TEST_F(EnvironmentBuilderTest, Function) {
  EXPECT_EQ(
      "Module\n"
      "+--Function<Normal> $Variable@1002\n"
      "|  +--Name |foo|\n"
      "|  +--ParameterList\n"
      "|  |  +--BindingNameElement $Variable@1003\n"
      "|  |  |  +--Name |a|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement $Variable@1004\n"
      "|  |  |  +--Name |b|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  +--BlockStatement\n"
      "|  |  +--ReturnStatement\n"
      "|  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  +--ReferenceExpression $Variable@1003\n"
      "|  |  |  |  |  +--Name |a|\n"
      "|  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  +--ReferenceExpression $Variable@1004\n"
      "|  |  |  |  |  +--Name |b|\n",
      RunOn("function foo(a, b) { return a + b; }"));
}

TEST_F(EnvironmentBuilderTest, FunctionAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement $Variable@1002\n"
      "|  |  +--Name |a|\n"
      "|  |  +--Function<Normal> $Function@1001\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("var a = function() {};"));
}

TEST_F(EnvironmentBuilderTest, Global) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression $Property@1001\n"
      "|  |  |  +--ReferenceExpression $Variable@1\n"
      "|  |  |  |  +--Name |global|\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n",
      RunOn("global.foo = 1;"));
}

TEST_F(EnvironmentBuilderTest, Let) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement $Variable@1001\n"
      "|  |  +--Name |a|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement $Variable@1002\n"
      "|  |  +--Name |b|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("let a = 1, b = 2;"));
}

TEST_F(EnvironmentBuilderTest, MemberExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> $Variable@1002\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@const|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--MemberExpression $Property@1004\n"
      "|  |  |  +--MemberExpression $Property@1003\n"
      "|  |  |  |  +--ReferenceExpression $Variable@1002\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |bar|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo.prototype.bar\n"))
      << "Old style class externs";
}

TEST_F(EnvironmentBuilderTest, Super) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |super|\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n",
      RunOn("super.foo = 1;"))
      << "'super' has no value";

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
      "|  +--ExpressionStatement\n"
      "|  |  +--AssignmentExpression<=>\n"
      "|  |  |  +--MemberExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |super|\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  +--Punctuator |=|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      RunOn("/** @type {number} */ super.foo = 1;"))
      << "'super' has no value";
}

TEST_F(EnvironmentBuilderTest, This) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |this|\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n",
      RunOn("this.foo = 1;"))
      << "'this' has no value";

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
      "|  +--ExpressionStatement\n"
      "|  |  +--AssignmentExpression<=>\n"
      "|  |  |  +--MemberExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |this|\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  +--Punctuator |=|\n"
      "|  |  |  +--NumericLiteral |1|\n",
      RunOn("/** @type {number} */ this.foo = 1;"))
      << "'this' has no value";
}

TEST_F(EnvironmentBuilderTest, Type) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement $Variable@1001\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement $Variable@1002\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @interface */ var Foo;\n"
            "/** @type {!Foo} */ var foo;\n"));

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
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement $Variable@1001\n"
      "|  |  |  +--Name |x|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {number} */ var x;"));

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeApplication\n"
      "|  |  |  |  +--TypeName\n"
      "|  |  |  |  |  +--Name |Array|\n"
      "|  |  |  |  +--Tuple\n"
      "|  |  |  |  |  +--TypeName\n"
      "|  |  |  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ConstStatement\n"
      "|  |  +--BindingNameElement $Variable@1001\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {Array<T>} @template T */ const foo;"));
}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/analyzer/name_resolver.h"

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
// NameResolverTest
//
class NameResolverTest : public AnalyzerTestBase {
 protected:
  NameResolverTest() = default;
  ~NameResolverTest() override = default;

  std::string RunOn(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(NameResolverTest);
};

std::string NameResolverTest::RunOn(base::StringPiece script_text) {
  const auto& module = ParseAsModule(script_text);
  const auto& context = NewContext();
  {
    NameResolver resolver(context.get());
    resolver.RunOn(module);
  }
  std::ostringstream ostream;
  ostream << AsPrintableTree(*context, module) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(NameResolverTest, Catch) {
  EXPECT_EQ(
      "Module\n"
      "+--TryCatchStatement\n"
      "|  +--BlockStatement\n"
      "|  |  +--ExpressionStatement\n"
      "|  |  |  +--CallExpression\n"
      "|  |  |  |  +--ReferenceExpression\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  +--CatchClause\n"
      "|  |  +--BindingNameElement CatchVar[bar@1001]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BlockStatement\n"
      "|  |  |  +--ThrowStatement\n"
      "|  |  |  |  +--ReferenceExpression CatchVar[bar@1001]\n"
      "|  |  |  |  |  +--Name |bar|\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNDEFINED_VARIABLE@6:9\n",
      RunOn("try { foo(); } catch (bar) { throw bar; }"));
}

TEST_F(NameResolverTest, Class) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1004]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> Function[baz@1006]\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { bar() {} baz() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
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
      "|  |  |  |  |  +--ReferenceExpression ParameterVar[x@1006]\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  +--JsDocText |*/|\n"
      "|  |  |  +--Method<NonStatic,Normal> Function[foo@1004]\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--ParameterList\n"
      "|  |  |  |  |  +--BindingNameElement ParameterVar[x@1006]\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { /** @param {number} x */ foo(x) {} }"));
}

TEST_F(NameResolverTest, ClassAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement VarVar[a@1003]\n"
      "|  |  +--Name |a|\n"
      "|  |  +--Class Class@1001\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--ObjectInitializer |{}|\n",
      RunOn("var a = class {};"));
}

TEST_F(NameResolverTest, ClassAnnotation) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Class Interface[Foo@1002]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--ObjectInitializer |{}|\n",
      RunOn("/** @interface */ class Foo {}"));

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {KEY@1001}\n"
      "|  |  |  |  +--Name |KEY|\n"
      "|  |  |  +--TypeName {VALUE@1002}\n"
      "|  |  |  |  +--Name |VALUE|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Class GenericClass<KEY,VALUE>[Map@1002]\n"
      "|  |  +--Name |Map|\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--ObjectInitializer |{}|\n",
      RunOn("/** @template KEY, VALUE */ class Map {}"));

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {KEY@1001}\n"
      "|  |  |  |  +--Name |KEY|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {VALUE@1002}\n"
      "|  |  |  |  +--Name |VALUE|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Class GenericClass<KEY,VALUE>[Map@1002]\n"
      "|  |  +--Name |Map|\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--ObjectInitializer |{}|\n",
      RunOn("/** @template KEY @template VALUE */ class Map {}"));
}

TEST_F(NameResolverTest, ClassError) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--NumericLiteral |1|\n"
      "ANALYZER_ERROR_ENVIRONMENT_EXPECT_METHOD@12:13\n",
      RunOn("class Foo { 1 }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1004]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1006]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@12:29\n",
      RunOn("class Foo { bar() {} bar() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Normal> Function[constructor@1004]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { static constructor() {} }"))
      << "We can use constructor as static method.";
}

TEST_F(NameResolverTest, ClassErrorConstructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Async> Function[constructor@1004]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:34\n",
      RunOn("class Foo { async constructor() {} }"))
      << "constructor can not be async.";

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1002]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Generator> Function[constructor@1004]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:29\n",
      RunOn("class Foo { *constructor() {} }"))
      << "constructor can not be generator.";
}

TEST_F(NameResolverTest, ComputedMemberExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Symbol@1003]\n"
      "|  |  +--Name |Symbol|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Foo@1007]\n"
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
      "|  |  +--ComputedMemberExpression PublicProperty[Symbol.iterator@1010]\n"
      "|  |  |  +--ReferenceExpression FunctionVar[Foo@1006]\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--MemberExpression PublicProperty[iterator@1009]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Symbol@1002]\n"
      "|  |  |  |  |  +--Name |Symbol|\n"
      "|  |  |  |  +--Name |iterator|\n",
      RunOn("/** @constructor */ function Symbol() {}\n"
            "/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo[Symbol.iterator]\n"));
}

TEST_F(NameResolverTest, ComputedMemberExpressionPrototype) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Symbol@1003]\n"
      "|  |  +--Name |Symbol|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Foo@1007]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ComputedMemberExpression PublicProperty[Symbol.iterator@1010]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1008]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1006]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--MemberExpression PublicProperty[iterator@1009]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Symbol@1002]\n"
      "|  |  |  |  |  +--Name |Symbol|\n"
      "|  |  |  |  +--Name |iterator|\n",
      RunOn("/** @constructor */ function Symbol() {}\n"
            "/** @constructor */ function Foo() {}\n"
            "/** @return {number} */ Foo.prototype[Symbol.iterator]\n"));
}

TEST_F(NameResolverTest, ConstError) {
  EXPECT_EQ(
      "Module\n"
      "+--ConstStatement\n"
      "|  +--BindingNameElement ConstVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement ConstVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |2|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@6:18\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@6:22\n",
      RunOn("const foo = 1, foo = 2;"));
}

TEST_F(NameResolverTest, ForwardRefernce) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {Foo@1001}\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001]\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1002]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[Foo@1003]\n",
      RunOn("/** @type {Foo} */ var foo;\n"
            "/** @constructor */ var Foo;\n"))
      << "Declarations are handled as if they are top of compilation unit";
}

TEST_F(NameResolverTest, Function) {
  EXPECT_EQ(
      "Module\n"
      "+--Function<Normal> Function[foo@1001]\n"
      "|  +--Name |foo|\n"
      "|  +--ParameterList\n"
      "|  |  +--BindingNameElement ParameterVar[a@1003]\n"
      "|  |  |  +--Name |a|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement ParameterVar[b@1004]\n"
      "|  |  |  +--Name |b|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  +--BlockStatement\n"
      "|  |  +--ReturnStatement\n"
      "|  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  +--ReferenceExpression ParameterVar[a@1003]\n"
      "|  |  |  |  |  +--Name |a|\n"
      "|  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  +--ReferenceExpression ParameterVar[b@1004]\n"
      "|  |  |  |  |  +--Name |b|\n",
      RunOn("function foo(a, b) { return a + b; }"));
}

TEST_F(NameResolverTest, FunctionAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement VarVar[a@1002]\n"
      "|  |  +--Name |a|\n"
      "|  |  +--Function<Normal> Function@1001\n"
      "|  |  |  +--Empty ||\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("var a = function() {};"));
}

TEST_F(NameResolverTest, FunctionAnnotation) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {KEY@1001}\n"
      "|  |  |  |  +--Name |KEY|\n"
      "|  |  |  +--TypeName {VALUE@1002}\n"
      "|  |  |  |  +--Name |VALUE|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> GenericClass<KEY,VALUE>[Map@1003]\n"
      "|  |  +--Name |Map|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n",
      RunOn("/** @constructor @template KEY, VALUE */ function Map() {}"));
}

TEST_F(NameResolverTest, Global) {
  EXPECT_EQ(
      "Module\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression PublicProperty[foo@1001]\n"
      "|  |  |  +--ReferenceExpression ConstVar[global@1]\n"
      "|  |  |  |  +--Name |global|\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n",
      RunOn("global.foo = 1;"));
}

TEST_F(NameResolverTest, GroupExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--ReturnStatement\n"
      "|  +--Annotation\n"
      "|  |  +--JsDocDocument\n"
      "|  |  |  +--JsDocText |/**|\n"
      "|  |  |  +--JsDocTag\n"
      "|  |  |  |  +--Name |@type|\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  |  +--GroupExpression\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |foo|\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNDEFINED_VARIABLE@29:32\n",
      RunOn("return /** @type {number} */(foo);"));
}

TEST_F(NameResolverTest, Let) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement LetVar[a@1001]\n"
      "|  |  +--Name |a|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement LetVar[b@1002]\n"
      "|  |  +--Name |b|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("let a = 1, b = 2;"));
}

TEST_F(NameResolverTest, LetError) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement LetVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement LetVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |2|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@4:16\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@4:20\n",
      RunOn("let foo = 1, foo = 2;"));
}

TEST_F(NameResolverTest, MemberExpression) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Foo@1003]\n"
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
      "|  |  +--MemberExpression PublicProperty[bar@1005]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |bar|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo.prototype.bar\n"))
      << "Old style class externs";

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {%symbol%}\n"
      "|  |  |  |  +--Name |symbol|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Function[Foo@1001]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@param|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--ReferenceExpression ParameterVar[x@1004]\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--MemberExpression PublicProperty[bar@1003]\n"
      "|  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--Name |bar|\n",
      RunOn("/** @return {symbol} */ function Foo() {}\n"
            "/** @param {number} x */ Foo.bar;\n"))
      << "Function declaration shortcut.";
}

TEST_F(NameResolverTest, Property) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Foo@1003]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression PublicProperty[bar@1005]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression PublicProperty[baz@1006]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "Foo.prototype.bar = 1;\n"
            "Foo.prototype.baz = 2;\n"))
      << "'Foo.prototype@1004' should be singleton.";

  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement VarVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--ElisionExpression ||\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression PublicProperty[bar@1003]\n"
      "|  |  |  +--MemberExpression PublicProperty[prop@1002]\n"
      "|  |  |  |  +--ReferenceExpression VarVar[foo@1001]\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--Name |prop|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression PublicProperty[baz@1004]\n"
      "|  |  |  +--MemberExpression PublicProperty[prop@1002]\n"
      "|  |  |  |  +--ReferenceExpression VarVar[foo@1001]\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--Name |prop|\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("var foo\n"
            "foo.prop.bar = 1;\n"
            "foo.prop.baz = 2;\n"))
      << "'foo.prop@1003' should be singleton.";
}

TEST_F(NameResolverTest, Prototype) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> GenericClass<T>[Foo@1003]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--MemberExpression PublicProperty[baz@1005]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |baz|\n",
      RunOn("/** @constructor @template T */ function Foo() {}\n"
            "/** @type {T} */ Foo.prototype.baz;\n"))
      << "Bind template parameter for 'Foo.prototyp.baz'";

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> GenericClass<T>[Foo@1003]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--ComputedMemberExpression\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Foo@1002]\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--ReferenceExpression\n"
      "|  |  |  |  +--Name |baz|\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNEXPECT_ANNOTATION@67:85\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNDEFINED_VARIABLE@81:84\n",
      RunOn("/** @constructor @template T */ function Foo() {}\n"
            "/** @type {T} */ Foo.prototype[baz];\n"))
      << "Bind template parameter for 'Foo.prototyp[baz]'";

  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Class[Map@1003]\n"
      "|  |  +--Name |Map|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {THIS@1002}\n"
      "|  |  |  |  +--Name |THIS|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {THIS@1002}\n"
      "|  |  |  |  +--Name |THIS|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--MemberExpression PublicProperty[set@1005]\n"
      "|  |  |  +--MemberExpression PublicProperty[prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression FunctionVar[Map@1002]\n"
      "|  |  |  |  |  +--Name |Map|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |set|\n",
      RunOn("/** @constructor */ function Map() {}\n"
            "/** @template THIS @return {THIS} */ Map.prototype.set;"));
}

TEST_F(NameResolverTest, StaticMember) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[Foo@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@private|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--ExpressionStatement\n"
      "|  |  +--MemberExpression PrivateProperty[bar@1004]\n"
      "|  |  |  +--ReferenceExpression VarVar[Foo@1001]\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--Name |bar|\n",
      RunOn("/** @constructor */ var Foo;\n"
            "/** @private @return {number} */ Foo.bar;"));
}

TEST_F(NameResolverTest, Super) {
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
      "|  |  |  +--TypeName {%number%}\n"
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

TEST_F(NameResolverTest, This) {
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
      "|  |  |  +--TypeName {%number%}\n"
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

TEST_F(NameResolverTest, Type) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Interface[Foo@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName {Foo@1001}\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1004]\n"
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
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[x@1001]\n"
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
      "|  |  |  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ConstStatement\n"
      "|  |  +--BindingNameElement ConstVar[foo@1001]\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNDEFINED_TYPE@11:16\n",
      RunOn("/** @type {Array<T>} @template T */ const foo;"))
      << "Later pass will report @template is unexpected for const.";
}

TEST_F(NameResolverTest, TypeAlias) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@typedef|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {Foo@1001}\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[bar@1002]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @typedef {number} */ var Foo;\n"
            "/** @type {Foo} */ var bar;\n"));
}

TEST_F(NameResolverTest, TypeApplication) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {T@1001}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || GenericInterface<T>[Foo@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@implements|\n"
      "|  |  |  +--TypeApplication\n"
      "|  |  |  |  +--TypeName {Foo<T>@1003}\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Tuple\n"
      "|  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Bar@1004]\n"
      "|  |  |  +--Name |Bar|\n"
      "|  |  |  +--ElisionExpression || Class[Bar@1005]\n",
      RunOn("/** @interface @template T */ var Foo;"
            "/** @constructor @implements {Foo<number>} */ var Bar;"));
}

TEST_F(NameResolverTest, VarConstructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[Foo@1002]\n",
      RunOn("/** @constructor */ var Foo;"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--Function<Normal> Class@1003\n"
      "|  |  |  |  +--Empty ||\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@param|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName {%anonymous%@1001}\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--ReferenceExpression ParameterVar[x@1007]\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1005]\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--Function<Normal> Function@1006\n"
      "|  |  |  |  +--Empty ||\n"
      "|  |  |  |  +--ParameterList\n"
      "|  |  |  |  |  +--BindingNameElement ParameterVar[x@1007]\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      RunOn("/** @constructor */ var Foo = function() {};\n"
            "/** @param {!Foo} x */ var foo = function(x) {};\n"));
}

TEST_F(NameResolverTest, VarConstructorMultiple) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[Foo@1002]\n"
      "|  |  +--BindingNameElement VarVar[Bar@1004]\n"
      "|  |  |  +--Name |Bar|\n"
      "|  |  |  +--ElisionExpression || Class[Bar@1005]\n",
      RunOn("/** @constructor */ var Foo, Bar;"))
      << "It is OK to define multiple constructors once.";
}

TEST_F(NameResolverTest, VarError) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement VarVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement VarVar[foo@1001]\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |2|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@4:16\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@4:20\n",
      RunOn("var foo = 1, foo = 2;"))
      << "It is legal to bind same variable multiple times with 'var', but we "
         "report error.";
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--ObjectBindingPattern\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_ENVIRONMENT_UNEXPECT_ANNOTATION@4:29\n",
      RunOn("/** @constructor */ var {Foo};"))
      << "Constructor should be a simple name.";
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@private|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001]\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_JSDOC_UNEXPECT_VISIBILITY@4:39\n",
      RunOn("/** @private @type {number} */ var Foo;"))
      << "Can not apply visibility";
}

}  // namespace analyzer
}  // namespace joana

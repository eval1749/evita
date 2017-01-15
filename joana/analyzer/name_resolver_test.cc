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
  NameResolver resolver(context.get());
  resolver.RunOn(module);
  std::ostringstream ostream;
  ostream << AsPrintableTree(*context, module) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(NameResolverTest, Class) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1002]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> Function[baz@1003]\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { bar() {} baz() {} }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
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
      "|  |  |  |  |  +--ReferenceExpression $x@1003\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  +--JsDocText |*/|\n"
      "|  |  |  +--Method<NonStatic,Normal> Function[foo@1002]\n"
      "|  |  |  |  +--Name |foo|\n"
      "|  |  |  |  +--ParameterList\n"
      "|  |  |  |  |  +--BindingNameElement $x@1003\n"
      "|  |  |  |  |  |  +--Name |x|\n"
      "|  |  |  |  |  |  +--ElisionExpression ||\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      RunOn("class Foo { /** @param {number} x */ foo(x) {} }"));
}

TEST_F(NameResolverTest, ClassAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement $a@1002\n"
      "|  |  +--Name |a|\n"
      "|  |  +--Class Class[%anonymous%@1001]\n"
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
      "|  +--Class Interface[Foo@1001]\n"
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
      "|  +--Class Class[Map<KEY,VALUE>@1001]\n"
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
      "|  +--Class Class[Map<KEY,VALUE>@1001]\n"
      "|  |  +--Name |Map|\n"
      "|  |  +--ElisionExpression ||\n"
      "|  |  +--ObjectInitializer |{}|\n",
      RunOn("/** @template KEY @template VALUE */ class Map {}"));
}

TEST_F(NameResolverTest, ClassError) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--NumericLiteral |1|\n"
      "ANALYZER_ERROR_ENVIRONMENT_EXPECT_METHOD@12:13\n",
      RunOn("class Foo { 1 }"));

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1002]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "|  |  +--Method<NonStatic,Normal> Function[bar@1003]\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@12:29\n",
      RunOn("class Foo { bar() {} bar() {} }"));
}

TEST_F(NameResolverTest, ClassErrorConstructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Async> Function[constructor@1002]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:34\n",
      RunOn("class Foo { async constructor() {} }"))
      << "constructor can not be async.";

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<NonStatic,Generator> Function[constructor@1002]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:29\n",
      RunOn("class Foo { *constructor() {} }"))
      << "constructor can not be generator.";

  EXPECT_EQ(
      "Module\n"
      "+--Class Class[Foo@1001]\n"
      "|  +--Name |Foo|\n"
      "|  +--ElisionExpression ||\n"
      "|  +--ObjectInitializer\n"
      "|  |  +--Method<Static,Normal> Function[constructor@1002]\n"
      "|  |  |  +--Name |constructor|\n"
      "|  |  |  +--ParameterList |()|\n"
      "|  |  |  +--BlockStatement |{}|\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@12:35\n",
      RunOn("class Foo { static constructor() {} }"))
      << "constructor can not be static.";
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
      "|  +--Function<Normal> Class[Foo@1002]\n"
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
      "|  |  +--ComputedMemberExpression [Symbol.iterator@1006]\n"
      "|  |  |  +--ReferenceExpression $Foo@1003\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  +--MemberExpression [iterator@1005]\n"
      "|  |  |  |  +--ReferenceExpression $Symbol@1004\n"
      "|  |  |  |  |  +--Name |Symbol|\n"
      "|  |  |  |  +--Name |iterator|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo[Symbol.iterator]\n"));
}

TEST_F(NameResolverTest, ConstError) {
  EXPECT_EQ(
      "Module\n"
      "+--ConstStatement\n"
      "|  +--BindingNameElement $foo@1001\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement $foo@1001\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |2|\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@6:18\n"
      "ANALYZER_ERROR_ENVIRONMENT_MULTIPLE_OCCURRENCES@6:22\n",
      RunOn("const foo = 1, foo = 2;"));
}

TEST_F(NameResolverTest, Function) {
  EXPECT_EQ(
      "Module\n"
      "+--Function<Normal> Function[foo@1001]\n"
      "|  +--Name |foo|\n"
      "|  +--ParameterList\n"
      "|  |  +--BindingNameElement $a@1003\n"
      "|  |  |  +--Name |a|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement $b@1004\n"
      "|  |  |  +--Name |b|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  +--BlockStatement\n"
      "|  |  +--ReturnStatement\n"
      "|  |  |  +--BinaryExpression<+>\n"
      "|  |  |  |  +--ReferenceExpression $a@1003\n"
      "|  |  |  |  |  +--Name |a|\n"
      "|  |  |  |  +--Punctuator |+|\n"
      "|  |  |  |  +--ReferenceExpression $b@1004\n"
      "|  |  |  |  |  +--Name |b|\n",
      RunOn("function foo(a, b) { return a + b; }"));
}

TEST_F(NameResolverTest, FunctionAnonymous) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement $a@1002\n"
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
      "|  +--Function<Normal> Class[Map<KEY,VALUE>@1002]\n"
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
      "|  |  +--MemberExpression [foo@1001]\n"
      "|  |  |  +--ReferenceExpression $global@1\n"
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
      "|  |  |  +--ReferenceExpression $foo@1001\n"
      "|  |  |  |  +--Name |foo|\n",
      RunOn("return /** @type {number} */(foo);"));
}

TEST_F(NameResolverTest, Let) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement $a@1001\n"
      "|  |  +--Name |a|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement $b@1002\n"
      "|  |  +--Name |b|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("let a = 1, b = 2;"));
}

TEST_F(NameResolverTest, LetError) {
  EXPECT_EQ(
      "Module\n"
      "+--LetStatement\n"
      "|  +--BindingNameElement $foo@1001\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement $foo@1001\n"
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
      "|  +--Function<Normal> Class[Foo@1002]\n"
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
      "|  |  +--MemberExpression [bar@1005]\n"
      "|  |  |  +--MemberExpression [prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1003\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |bar|\n",
      RunOn("/** @constructor */ function Foo() {}\n"
            "/** @const */ Foo.prototype.bar\n"))
      << "Old style class externs";
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
      "|  +--Function<Normal> Class[Foo@1002]\n"
      "|  |  +--Name |Foo|\n"
      "|  |  +--ParameterList |()|\n"
      "|  |  +--BlockStatement |{}|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression [bar@1005]\n"
      "|  |  |  +--MemberExpression [prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1003\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression [baz@1006]\n"
      "|  |  |  +--MemberExpression [prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1003\n"
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
      "|  +--BindingNameElement $foo@1001\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--ElisionExpression ||\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression [bar@1004]\n"
      "|  |  |  +--MemberExpression [prop@1003]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1002\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prop|\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |1|\n"
      "+--ExpressionStatement\n"
      "|  +--AssignmentExpression<=>\n"
      "|  |  +--MemberExpression [baz@1005]\n"
      "|  |  |  +--MemberExpression [prop@1003]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1002\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prop|\n"
      "|  |  |  +--Name |baz|\n"
      "|  |  +--Punctuator |=|\n"
      "|  |  +--NumericLiteral |2|\n",
      RunOn("var foo\n"
            "Foo.prop.bar = 1;\n"
            "Foo.prop.baz = 2;\n"))
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
      "|  +--Function<Normal> Class[Foo<T>@1002]\n"
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
      "|  |  +--MemberExpression [baz@1005]\n"
      "|  |  |  +--MemberExpression [prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1003\n"
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
      "|  +--Function<Normal> Class[Foo<T>@1002]\n"
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
      "|  |  |  +--MemberExpression [prototype@1004]\n"
      "|  |  |  |  +--ReferenceExpression $Foo@1003\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Name |prototype|\n"
      "|  |  |  +--ReferenceExpression $baz@1005\n"
      "|  |  |  |  +--Name |baz|\n",
      RunOn("/** @constructor @template T */ function Foo() {}\n"
            "/** @type {T} */ Foo.prototype[baz];\n"))
      << "Bind template parameter for 'Foo.prototyp[baz]'";
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
      "|  +--JsDocDocument Interface[%anonymous%@1002]\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement $Foo@1001\n"
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
      "|  |  +--BindingNameElement $foo@1003\n"
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
      "|  |  +--BindingNameElement $x@1001\n"
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
      "|  |  |  +--TypeName\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--ConstStatement\n"
      "|  |  +--BindingNameElement $foo@1001\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {Array<T>} @template T */ const foo;"))
      << "Later pass will report @template is unexpected for const.";
}

TEST_F(NameResolverTest, VarConstructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument Class[%anonymous%@1002]\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement $Foo@1001\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @constructor */ var Foo;"));
}

TEST_F(NameResolverTest, VarError) {
  EXPECT_EQ(
      "Module\n"
      "+--VarStatement\n"
      "|  +--BindingNameElement $foo@1001\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--NumericLiteral |1|\n"
      "|  +--BindingNameElement $foo@1001\n"
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
      "|  |  +--BindingNameElement $Foo@1001\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "|  |  +--BindingNameElement $Bar@1002\n"
      "|  |  |  +--Name |Bar|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@20:33\n",
      RunOn("/** @constructor */ var Foo, Bar;"))
      << "Constructor should be for single name.";

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
      "|  |  |  +--BindingNameElement $Foo@1001\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_ENVIRONMENT_INVALID_CONSTRUCTOR@20:30\n",
      RunOn("/** @constructor */ var {Foo};"))
      << "Constructor should be a simple name.";
}

}  // namespace analyzer
}  // namespace joana

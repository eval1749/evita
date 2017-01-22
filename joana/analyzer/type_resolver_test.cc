// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/analyzer/type_resolver.h"

#include "joana/analyzer/analyzer_test_base.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/name_resolver.h"
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
// TypeResolverTest
//
class TypeResolverTest : public AnalyzerTestBase {
 protected:
  TypeResolverTest() = default;
  ~TypeResolverTest() override = default;

  std::string RunOn(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeResolverTest);
};

std::string TypeResolverTest::RunOn(base::StringPiece script_text) {
  const auto& module = ParseAsModule(script_text);
  const auto& context = NewContext();
  {
    NameResolver name_resolver(context.get());
    name_resolver.RunOn(module);
  }
  {
    TypeResolver type_resolver(context.get());
    type_resolver.RunOn(module);
  }
  std::ostringstream ostream;
  ostream << AsPrintableTree(*context, module) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(TypeResolverTest, AnyType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--AnyType |*|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {*}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {*} */ var foo;"));
}

TEST_F(TypeResolverTest, Constructor) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001] "
      "{function(new:class@1001):class@1001}\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[%anonymous%@1002]\n",
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
      "|  |  +--BindingNameElement VarVar[Foo@1001] "
      "{function(new:class@1001):class@1001}\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--Function<Normal> Class[%anonymous%@1003]\n"
      "|  |  |  |  +--Empty ||\n"
      "|  |  |  |  +--ParameterList |()|\n"
      "|  |  |  |  +--BlockStatement |{}|\n",
      RunOn("/** @constructor */ var Foo = function() {};"));
}

TEST_F(TypeResolverTest, Function) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@param|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--ReferenceExpression ParameterVar[x@1003]\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--JsDocText ||\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  +--Name |string|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--Function<Normal> Function[foo@1001] "
      "{function(%number%):%string%}\n"
      "|  |  +--Name |foo|\n"
      "|  |  +--ParameterList\n"
      "|  |  |  +--BindingNameElement ParameterVar[x@1003]\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  |  +--ElisionExpression ||\n"
      "|  |  +--BlockStatement |{}|\n",
      RunOn("/** @param {number} x @return {string} */ function foo(x) {}"));
}

TEST_F(TypeResolverTest, InvalidType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--InvalidType |+|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {?}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {+} */ var foo"));
}

TEST_F(TypeResolverTest, NonNullableType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {%number%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_JSDOC_EXPECT_NULLABLE_TYPE@12:18\n",
      RunOn("/** @type {!number} */ var foo"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001] "
      "{function(new:class@1001):class@1001}\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[%anonymous%@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeName {class@1001}\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[bar@1003] {class@1001}\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @constructor */ var Foo;\n"
            "/** @type {!Foo} */ var bar;\n"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--FunctionType<Normal>\n"
      "|  |  |  |  |  +--Tuple |()|\n"
      "|  |  |  |  |  +--VoidType ||\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {function(this:?)}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_JSDOC_EXPECT_NULLABLE_TYPE@12:22\n",
      RunOn("/** @type {!function()} */ var foo"));
}

TEST_F(TypeResolverTest, NullableType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NullableType\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {%null%|%number%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {?number} */ var foo"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001] "
      "{function(new:class@1001):class@1001}\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Class[%anonymous%@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TypeName {class@1001}\n"
      "|  |  |  |  +--Name |Foo|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[bar@1003] {%null%|class@1001}\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @constructor */ var Foo;\n"
            "/** @type {Foo} */ var bar;\n"))
      << "Non-primitive type reference is nullable";
}

TEST_F(TypeResolverTest, OptionalType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@param|\n"
      "|  |  |  +--OptionalType\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  +--ReferenceExpression ParameterVar[@1001] {%number%}\n"
      "|  |  |  |  +--Name ||\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1002] {function(%number%)}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @param {number=} */ var foo"));
}

TEST_F(TypeResolverTest, RecordType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--RecordType\n"
      "|  |  |  |  +--Property\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  +--Name |baz|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[quux@1001] "
      "{%null%|{foo:%number%,baz:*}}\n"
      "|  |  |  +--Name |quux|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {{foo: number, baz}} */ var quux;"));
}

TEST_F(TypeResolverTest, RecordTypeError) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--RecordType\n"
      "|  |  |  |  +--Property\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  +--Property\n"
      "|  |  |  |  |  +--Name |foo|\n"
      "|  |  |  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  |  |  +--Name |string|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[quux@1001] {%null%|{foo:%number%}}\n"
      "|  |  |  +--Name |quux|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_JSDOC_MULTIPLE_PROPERTY@12:28\n",
      RunOn("/** @type {{foo: number, foo: string}} */ var quux;"));
}

TEST_F(TypeResolverTest, TupleType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--TupleType\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  |  +--Name |string|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {[%number%,%string%]}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {[number,string]} */ var foo"));
}

TEST_F(TypeResolverTest, TypeAlias) {
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
      "|  |  +--BindingNameElement VarVar[Foo@1001] {%number%}\n"
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
      "|  |  +--BindingNameElement VarVar[bar@1002] {Foo@1001}\n"
      "|  |  |  +--Name |bar|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @typedef {number} */ var Foo;\n"
            "/** @type {Foo} */ var bar;\n"));
}

TEST_F(TypeResolverTest, TypeGroup) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NullableType\n"
      "|  |  |  |  +--TypeGroup\n"
      "|  |  |  |  |  +--UnionType\n"
      "|  |  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  |  |  |  +--Name |string|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] "
      "{%null%|%number%|%string%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {?(number|string)} */ var foo;"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--NonNullableType\n"
      "|  |  |  |  +--TypeGroup\n"
      "|  |  |  |  |  +--UnionType\n"
      "|  |  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  |  |  |  +--Name |string|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {%number%|%string%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n"
      "ANALYZER_ERROR_JSDOC_EXPECT_NULLABLE_TYPE@12:27\n",
      RunOn("/** @type {!(number|string)} */ var foo;"));
}

TEST_F(TypeResolverTest, TypeApplication) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@interface|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@template|\n"
      "|  |  |  +--TypeName {T@1002}\n"
      "|  |  |  |  +--Name |T|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Foo@1001] {class<T>@1003}\n"
      "|  |  |  +--Name |Foo|\n"
      "|  |  |  +--ElisionExpression || Interface[%anonymous%<T>@1002]\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@constructor|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@implements|\n"
      "|  |  |  +--TypeApplication\n"
      "|  |  |  |  +--TypeName {class<T>@1003}\n"
      "|  |  |  |  |  +--Name |Foo|\n"
      "|  |  |  |  +--Tuple\n"
      "|  |  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  |  +--Name |number|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[Bar@1003] "
      "{function(new:class@1004):class@1004}\n"
      "|  |  |  +--Name |Bar|\n"
      "|  |  |  +--ElisionExpression || Class[%anonymous%@1004]\n",
      RunOn("/** @interface @template T */ var Foo;"
            "/** @constructor @implements {Foo<number>} */ var Bar;"));
}

TEST_F(TypeResolverTest, UnionType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--UnionType\n"
      "|  |  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  |  +--Name |number|\n"
      "|  |  |  |  +--TypeName {%string%}\n"
      "|  |  |  |  |  +--Name |string|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {%number%|%string%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {number|string} */ var foo"));
}

TEST_F(TypeResolverTest, UnknownType) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@type|\n"
      "|  |  |  +--UnknownType |?|\n"
      "|  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {*}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {?} */ var foo;"));
}

TEST_F(TypeResolverTest, Var) {
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
      "|  |  +--BindingNameElement VarVar[foo@1001] {%number%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @type {number} */ var foo;"));
}

TEST_F(TypeResolverTest, VarFunction) {
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@param|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--ReferenceExpression ParameterVar[x@1001] {%number%}\n"
      "|  |  |  |  +--Name |x|\n"
      "|  |  |  +--JsDocText |*/|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1002] {function(%number%)}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @param {number} x */ var foo;"));
  EXPECT_EQ(
      "Module\n"
      "+--Annotation\n"
      "|  +--JsDocDocument\n"
      "|  |  +--JsDocText |/**|\n"
      "|  |  +--JsDocTag\n"
      "|  |  |  +--Name |@return|\n"
      "|  |  |  +--TypeName {%number%}\n"
      "|  |  |  |  +--Name |number|\n"
      "|  |  |  +--JsDocText |x */|\n"
      "|  +--VarStatement\n"
      "|  |  +--BindingNameElement VarVar[foo@1001] {function():%number%}\n"
      "|  |  |  +--Name |foo|\n"
      "|  |  |  +--ElisionExpression ||\n",
      RunOn("/** @return {number} x */ var foo;"));
}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/analyzer/environment_builder.h"

#include "joana/analyzer/analyzer_test_base.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/value.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"
#include "joana/base/escaped_string_piece.h"
#include "joana/parser/public/parse.h"
#include "joana/testing/simple_error_sink.h"

namespace joana {
namespace analyzer {

namespace {

struct Printable {
  const Value* value;
};

Printable AsPrintable(const Value& value) {
  return Printable{&value};
}

std::ostream& operator<<(std::ostream& ostream, const Printable& printable) {
  const auto& value = *printable.value;
  const auto& range = value.node().range();
  return ostream << value.class_name() << '@' << value.id() << '['
                 << range.start() << '-' << range.end() << "] "
                 << EscapedStringPiece16(range.GetString(), '|', 20);
}

}  // namespace

//
// EnvironmentBuilderTest
//
class EnvironmentBuilderTest : public AnalyzerTestBase {
 protected:
  EnvironmentBuilderTest() = default;
  ~EnvironmentBuilderTest() override = default;

  std::string ListValues(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilderTest);
};

std::string EnvironmentBuilderTest::ListValues(base::StringPiece script_text) {
  const auto& module = ParseAsModule(script_text);
  EnvironmentBuilder builder(&analyzer_context());
  builder.RunOn(module);
  std::ostringstream ostream;
  for (const auto& node : ast::NodeTraversal::DescendantsOf(module)) {
    const auto* value = analyzer_context().TryValueOf(node);
    if (!value)
      continue;
    ostream << node.syntax() << '[' << node.range().start() << '-'
            << node.range().end() << "]=" << AsPrintable(*value) << std::endl;
  }
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(EnvironmentBuilderTest, Class) {
  EXPECT_EQ(
      "Class[0-31]=Variable@1002[6-9] |Foo|\n"
      "Method<NonStatic,Normal>[12-20]=Function@1006[12-20] |bar() {}|\n"
      "Method<NonStatic,Normal>[21-29]=Function@1008[21-29] |baz() {}|\n",
      ListValues("class Foo { bar() {} baz() {} }"));
}

TEST_F(EnvironmentBuilderTest, ClassAnonymous) {
  EXPECT_EQ(
      "BindingNameElement[4-16]=Variable@1003[4-5] |a|\n"
      "Class[8-16]=Function@1001[8-16] |class {}|\n",
      ListValues("var a = class {};"));
}

TEST_F(EnvironmentBuilderTest, ClassError) {
  EXPECT_EQ(
      "Class[0-15]=Variable@1002[6-9] |Foo|\n"
      "ANALYZER_ERROR_ENVIRONMENT_EXPECT_METHOD@12:13\n",
      ListValues("class Foo { 1 }"));
}

TEST_F(EnvironmentBuilderTest, ComputedMemberExpression) {
  EXPECT_EQ(
      "Function<Normal>[20-37]=Variable@1002[29-32] |Foo|\n"
      "ComputedMemberExpression[52-72]=Property@1006[56-71] |Symbol.iterator|\n"
      "ReferenceExpression[52-55]=Variable@1002[29-32] |Foo|\n"
      "MemberExpression[56-71]=Property@1005[63-71] |iterator|\n"
      "ReferenceExpression[56-62]=Variable@1004[56-62] |Symbol|\n",
      ListValues("/** @constructor */ function Foo() {}\n"
                 "/** @const */ Foo[Symbol.iterator]\n"));
}

TEST_F(EnvironmentBuilderTest, Function) {
  EXPECT_EQ(
      "Function<Normal>[0-36]=Variable@1002[9-12] |foo|\n"
      "BindingNameElement[13-14]=Variable@1003[13-14] |a|\n"
      "BindingNameElement[16-17]=Variable@1004[16-17] |b|\n"
      "ReferenceExpression[28-29]=Variable@1003[13-14] |a|\n"
      "ReferenceExpression[32-33]=Variable@1004[16-17] |b|\n",
      ListValues("function foo(a, b) { return a + b; }"));
}

TEST_F(EnvironmentBuilderTest, FunctionAnonymous) {
  EXPECT_EQ(
      "BindingNameElement[4-21]=Variable@1002[4-5] |a|\n"
      "Function<Normal>[8-21]=Function@1001[8-21] |function() {}|\n",
      ListValues("var a = function() {};"));
}

TEST_F(EnvironmentBuilderTest, Global) {
  EXPECT_EQ(
      "MemberExpression[0-10]=Property@1001[7-10] |foo|\n"
      "ReferenceExpression[0-6]=Variable@15[8-14] |global|\n",
      ListValues("global.foo = 1;"));
}

TEST_F(EnvironmentBuilderTest, Let) {
  EXPECT_EQ(
      "BindingNameElement[4-9]=Variable@1001[4-5] |a|\n"
      "BindingNameElement[11-16]=Variable@1002[11-12] |b|\n",
      ListValues("let a = 1, b = 2;"));
}

TEST_F(EnvironmentBuilderTest, MemberExpression) {
  EXPECT_EQ(
      "Function<Normal>[20-37]=Variable@1002[29-32] |Foo|\n"
      "MemberExpression[52-69]=Property@1005[66-69] |bar|\n"
      "MemberExpression[52-65]=Property@1004[56-65] |prototype|\n"
      "ReferenceExpression[52-55]=Variable@1002[29-32] |Foo|\n",
      ListValues("/** @constructor */ function Foo() {}\n"
                 "/** @const */ Foo.prototype.bar\n"))
      << "Old style class externs";
}

TEST_F(EnvironmentBuilderTest, Super) {
  EXPECT_EQ("", ListValues("super.foo = 1;")) << "'super' has no value";

  EXPECT_EQ("TypeName[11-17]=PrimitiveType@6[20-26] |number|\n",
            ListValues("/** @type {number} */ super.foo = 1;"))
      << "'super' has no value";
}

TEST_F(EnvironmentBuilderTest, This) {
  EXPECT_EQ("", ListValues("this.foo = 1;")) << "'this' has no value";

  EXPECT_EQ("TypeName[11-17]=PrimitiveType@6[20-26] |number|\n",
            ListValues("/** @type {number} */ this.foo = 1;"))
      << "'this' has no value";
}

TEST_F(EnvironmentBuilderTest, Type) {
  EXPECT_EQ(
      "BindingNameElement[22-25]=Variable@1001[22-25] |Foo|\n"
      "TypeName[39-42]=TypeReference@1003[22-25] |Foo|\n"
      "BindingNameElement[51-54]=Variable@1002[51-54] |foo|\n",
      ListValues("/** @interface */ var Foo;\n"
                 "/** @type {!Foo} */ var foo;\n"));

  EXPECT_EQ(
      "TypeName[11-17]=PrimitiveType@6[20-26] |number|\n"
      "BindingNameElement[26-27]=Variable@1004[26-27] |x|\n",
      ListValues("/** @type {number} */ var x;"));
}

}  // namespace analyzer
}  // namespace joana

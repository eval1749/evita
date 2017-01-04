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
#include "joana/ast/syntax.h"
#include "joana/ast/syntax_visitor.h"
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

//
// ValueSink
//
class ValueSink final : public ast::SyntaxVisitor {
 public:
  explicit ValueSink(std::ostream* ostream, const Context& context);
  ~ValueSink() = default;

 private:
  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node);

  const Context& context_;
  std::ostream* const ostream_;

  DISALLOW_COPY_AND_ASSIGN(ValueSink);
};

ValueSink::ValueSink(std::ostream* ostream, const Context& context)
    : context_(context), ostream_(ostream) {}

void ValueSink::VisitDefault(const ast::Node& node) {
  const auto* value = context_.TryValueOf(node);
  if (!value)
    return;
  *ostream_ << node.syntax() << '[' << node.range().start() << '-'
            << node.range().end() << "]=" << AsPrintable(*value) << std::endl;
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
  ValueSink sink(&ostream, analyzer_context());
  DepthFirstTraverse(&sink, module);
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

TEST_F(EnvironmentBuilderTest, Class) {
  EXPECT_EQ(
      "Class[0-31]=Class@2[0-31] |class Foo { bar() {}...|\n"
      "ObjectInitializer[10-31]=OrdinaryObject@1[10-31] |{ bar() {} baz() {} "
      "...|\n"
      "Method<NonStatic,Normal>[12-20]=Method@3[12-20] |bar() {}|\n"
      "Method<NonStatic,Normal>[21-29]=Method@4[21-29] |baz() {}|\n",
      ListValues("class Foo { bar() {} baz() {} }"));
}

TEST_F(EnvironmentBuilderTest, ClassError) {
  EXPECT_EQ(
      "Class[0-15]=Class@2[0-15] |class Foo { 1 }|\n"
      // Foo.prototype
      "ObjectInitializer[10-15]=OrdinaryObject@1[10-15] |{ 1 }|\n"
      "ANALYZER_ERROR_ENVIRONMENT_EXPECT_METHOD@12:13\n",
      ListValues("class Foo { 1 }"));
}

TEST_F(EnvironmentBuilderTest, Function) {
  EXPECT_EQ(
      "Function<Normal>[0-36]=Function@1[0-36] |function foo(a, b) {...|\n"
      "BindingNameElement[13-14]=Variable@2[13-14] |a|\n"
      "BindingNameElement[16-17]=Variable@3[16-17] |b|\n"
      "ReferenceExpression[28-29]=Variable@2[13-14] |a|\n"
      "ReferenceExpression[32-33]=Variable@3[16-17] |b|\n",
      ListValues("function foo(a, b) { return a + b; }"));
}

TEST_F(EnvironmentBuilderTest, Let) {
  EXPECT_EQ(
      "BindingNameElement[4-9]=Variable@1[4-9] |a = 1|\n"
      "BindingNameElement[11-16]=Variable@2[11-16] |b = 2|\n",
      ListValues("let a = 1, b = 2;"));
}

TEST_F(EnvironmentBuilderTest, MemberExpression) {
  EXPECT_EQ(
      "Name[4-16]=OrdinaryObject@1[4-16] |@constructor|\n"  // prototype
      "Function<Normal>[20-37]=Class@2[20-37] |function Foo() {}|\n"
      "MemberExpression[52-65]=OrdinaryObject@1[4-16] |@constructor|\n"
      "ReferenceExpression[52-55]=Class@2[20-37] |function Foo() {}|\n"
      "Name[66-69]=Variable@3[66-69] |bar|\n",
      ListValues("/** @constructor */ function Foo() {}\n"
                 "/** @const */ Foo.prototype.bar\n"))
      << "Old style class externs";
}

}  // namespace analyzer
}  // namespace joana
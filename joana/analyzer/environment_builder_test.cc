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
#include "joana/ast/syntax_visitor.h"
#include "joana/base/escaped_string_piece.h"
#include "joana/parser/public/parse.h"

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
  explicit ValueSink(const Context& context);
  ~ValueSink() = default;

  std::string GetResult() const { return ostream_.str(); }

 private:
  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node);

  const Context& context_;
  std::ostringstream ostream_;

  DISALLOW_COPY_AND_ASSIGN(ValueSink);
};

ValueSink::ValueSink(const Context& context) : context_(context) {}

void ValueSink::VisitDefault(const ast::Node& node) {
  const auto* value = context_.TryValueOf(node);
  if (!value)
    return;
  ostream_ << AsPrintable(*value) << std::endl;
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
  ValueSink sink(analyzer_context());
  DepthFirstTraverse(&sink, module);
  return sink.GetResult();
}

TEST_F(EnvironmentBuilderTest, Class) {
  EXPECT_EQ(
      "Class@1[0-31] |class Foo { bar() {}...|\n"
      "Function@2[12-20] |bar() {}|\n"
      "Function@3[21-29] |baz() {}|\n",
      ListValues("class Foo { bar() {} baz() {} }"));
}

TEST_F(EnvironmentBuilderTest, Function) {
  EXPECT_EQ(
      "Function@1[0-36] |function foo(a, b) {...|\n"
      "Variable@2[13-14] |a|\n"
      "Variable@3[16-17] |b|\n"
      "Variable@2[13-14] |a|\n"
      "Variable@3[16-17] |b|\n",
      ListValues("function foo(a, b) { return a + b; }"));
}

TEST_F(EnvironmentBuilderTest, Let) {
  EXPECT_EQ(
      "Variable@1[4-9] |a = 1|\n"
      "Variable@2[11-16] |b = 2|\n",
      ListValues("let a = 1, b = 2;"));
}

}  // namespace analyzer
}  // namespace joana

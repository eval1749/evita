// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/ir/node.h"

#include "joana/base/memory/zone.h"
#include "joana/ir/node_factory.h"
#include "joana/ir/operator_factory.h"
#include "joana/ir/type.h"
#include "joana/ir/type_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace ir {

namespace {

std::string ToString(const Node& node) {
  std::ostringstream ostream;
  ostream << node;
  return ostream.str();
}

}  // namespace

//
// IrNodeTest
//
class IrNodeTest : public ::testing::Test {
 public:
  ~IrNodeTest() override = default;

 protected:
  IrNodeTest();

  NodeFactory& node_factory() { return node_factory_; }
  OperatorFactory& operator_factory() { return operator_factory_; }
  TypeFactory& type_factory() { return type_factory_; }

 private:
  Zone zone_;
  OperatorFactory operator_factory_;
  TypeFactory type_factory_;
  NodeFactory node_factory_;

  DISALLOW_COPY_AND_ASSIGN(IrNodeTest);
};

IrNodeTest::IrNodeTest()
    : zone_("IrNodeTest"),
      operator_factory_(&zone_),
      type_factory_(&zone_),
      node_factory_(&zone_, &operator_factory_, &type_factory_) {}

TEST_F(IrNodeTest, If) {
  auto& output_type = type_factory().NewTupleType(
      type_factory().control_type(), type_factory().effect_type(),
      type_factory().NewTupleType());

  auto& start = node_factory().NewStartNode(output_type);
  auto& control = node_factory().NewProjectionNode(start, 0);

  auto& condition = node_factory().NewLiteralBool(true);
  auto& if_node = node_factory().NewIfNode(control, condition);
  node_factory().NewIfTrueNode(if_node);
  node_factory().NewIfTrueNode(if_node);

  EXPECT_EQ("4:If(%r2, true):Control", ToString(if_node));
}

TEST_F(IrNodeTest, LiteralBool) {
  auto& literal_true = node_factory().NewLiteralBool(true);
  auto& literal_false = node_factory().NewLiteralBool(false);

  EXPECT_EQ(0, literal_true.number_of_inputs());
  EXPECT_EQ(0, literal_false.number_of_inputs());

  EXPECT_NE(literal_true, literal_false);
  EXPECT_EQ(literal_true, node_factory().NewLiteralBool(true))
      << "We can share literal true node";
  EXPECT_EQ(literal_false, node_factory().NewLiteralBool(false))
      << "We can share literal false node";
  EXPECT_EQ("1:LiteralBool<1>():Bool", ToString(literal_true));
  EXPECT_EQ("2:LiteralBool<0>():Bool", ToString(literal_false));
}

TEST_F(IrNodeTest, LiteralFloat64) {
  auto& literal_123 = node_factory().NewLiteralFloat64(123);
  auto& literal_456 = node_factory().NewLiteralFloat64(456);

  EXPECT_EQ(0, literal_123.number_of_inputs());
  EXPECT_EQ(0, literal_456.number_of_inputs());

  EXPECT_NE(literal_123, literal_456);
  EXPECT_EQ(literal_123, node_factory().NewLiteralFloat64(123))
      << "We can share literal float64 node";
  EXPECT_EQ(literal_456, node_factory().NewLiteralFloat64(456))
      << "We can share literal float64 node";
  EXPECT_EQ("1:LiteralFloat64<123>():Float64", ToString(literal_123));
  EXPECT_EQ("2:LiteralFloat64<456>():Float64", ToString(literal_456));
}

TEST_F(IrNodeTest, LiteralInt64) {
  auto& literal_123 = node_factory().NewLiteralInt64(123);
  auto& literal_456 = node_factory().NewLiteralInt64(456);

  EXPECT_EQ(0, literal_123.number_of_inputs());
  EXPECT_EQ(0, literal_456.number_of_inputs());

  EXPECT_NE(literal_123, literal_456);
  EXPECT_EQ(literal_123, node_factory().NewLiteralInt64(123))
      << "We can share literal int64 node";
  EXPECT_EQ(literal_456, node_factory().NewLiteralInt64(456))
      << "We can share literal int64 node";
  EXPECT_EQ("1:LiteralInt64<123>():Int64", ToString(literal_123));
  EXPECT_EQ("2:LiteralInt64<456>():Int64", ToString(literal_456));
}

TEST_F(IrNodeTest, LiteralString) {
  auto& literal_abc = node_factory().NewLiteralString(L"abc");
  auto& literal_def = node_factory().NewLiteralString(L"def");

  EXPECT_EQ(0, literal_abc.number_of_inputs());
  EXPECT_EQ(0, literal_def.number_of_inputs());

  EXPECT_NE(literal_abc, literal_def);
  EXPECT_EQ(literal_abc, node_factory().NewLiteralString(L"abc"))
      << "We can share literal string node";
  EXPECT_EQ(literal_def, node_factory().NewLiteralString(L"def"))
      << "We can share literal string node";
  EXPECT_EQ("1:LiteralString<\"abc\">():String", ToString(literal_abc));
  EXPECT_EQ("2:LiteralString<\"def\">():String", ToString(literal_def));
}

TEST_F(IrNodeTest, Projection) {
  auto& literal_true = node_factory().NewLiteralBool(true);
  auto& literal_123 = node_factory().NewLiteralInt64(123);
  auto& literal_456 = node_factory().NewLiteralFloat64(456);
  auto& tuple =
      node_factory().NewTupleNode(literal_true, literal_123, literal_456);
  auto& projection0 = node_factory().NewProjectionNode(tuple, 0);
  auto& projection1 = node_factory().NewProjectionNode(tuple, 1);
  auto& projection2 = node_factory().NewProjectionNode(tuple, 2);

  EXPECT_EQ(1u, projection0.number_of_inputs());
  EXPECT_EQ(1u, projection1.number_of_inputs());
  EXPECT_EQ(1u, projection2.number_of_inputs());

  EXPECT_EQ(type_factory().bool_type(), projection0.output_type());
  EXPECT_EQ(type_factory().int64_type(), projection1.output_type());
  EXPECT_EQ(type_factory().float64_type(), projection2.output_type());

  EXPECT_EQ("6:Projection<1>(%t4):Int64", ToString(projection1));
}

TEST_F(IrNodeTest, Ret) {
  auto& output_type = type_factory().NewTupleType(
      type_factory().control_type(), type_factory().effect_type(),
      type_factory().NewTupleType());
  auto& literal_void = node_factory().NewLiteralVoid();
  auto& start = node_factory().NewStartNode(output_type);
  auto& control = node_factory().NewProjectionNode(start, 0);
  auto& ret = node_factory().NewRetNode(control, literal_void);

  EXPECT_EQ("4:Ret(%r3, void):Control", ToString(ret));
}

TEST_F(IrNodeTest, Start) {
  auto& output_type = type_factory().NewTupleType(
      type_factory().control_type(), type_factory().effect_type(),
      type_factory().NewTupleType());

  auto& start = node_factory().NewStartNode(output_type);
  EXPECT_EQ(0u, start.number_of_inputs());
  EXPECT_EQ("1:Start():Tuple<Control, Effect, Tuple<>>", ToString(start));
}

TEST_F(IrNodeTest, Tuple) {
  auto& literal_true = node_factory().NewLiteralBool(true);
  auto& literal_123 = node_factory().NewLiteralInt64(123);
  auto& literal_456 = node_factory().NewLiteralFloat64(456);
  auto& tuple =
      node_factory().NewTupleNode(literal_true, literal_123, literal_456);

  EXPECT_EQ(3u, tuple.number_of_inputs());
  EXPECT_EQ("4:Tuple<3>(true, 123, 456):Tuple<Bool, Int64, Float64>",
            ToString(tuple));
}

}  // namespace ir
}  // namespace joana

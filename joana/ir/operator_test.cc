// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ir/common_operators.h"

#include "joana/base/memory/zone.h"
#include "joana/ir/operator_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace ir {

//
// IrOperatorTest
//
class IrOperatorTest : public ::testing::Test {
 public:
  ~IrOperatorTest() override = default;

  OperatorFactory& factory() { return operator_factory_; }

 protected:
  IrOperatorTest();

 private:
  Zone zone_;
  OperatorFactory operator_factory_;

  DISALLOW_COPY_AND_ASSIGN(IrOperatorTest);
};

IrOperatorTest::IrOperatorTest()
    : zone_("IrOperatorTest"), operator_factory_(&zone_) {}

TEST_F(IrOperatorTest, LiteralBool) {
  auto& true_op = factory().NewLiteralBool(true);
  auto& false_op = factory().NewLiteralBool(false);
  EXPECT_EQ(true, true_op.As<LiteralBoolOperator>().data());
  EXPECT_EQ(false, false_op.As<LiteralBoolOperator>().data());
  EXPECT_NE(true_op, false_op);
  EXPECT_EQ(&factory().NewLiteralBool(true), &true_op);
  EXPECT_EQ(&factory().NewLiteralBool(false), &false_op);
}

TEST_F(IrOperatorTest, LiteralFloat64) {
  EXPECT_EQ(factory().NewLiteralFloat64(0), factory().NewLiteralFloat64(0));
  EXPECT_NE(factory().NewLiteralFloat64(0), factory().NewLiteralFloat64(1));
}

TEST_F(IrOperatorTest, LiteralInt64) {
  auto& lit0 = factory().NewLiteralInt64(0);
  auto& lit1 = factory().NewLiteralInt64(1);

  EXPECT_EQ(factory().NewLiteralInt64(0), lit0);
  EXPECT_NE(lit1, lit0);
  EXPECT_TRUE(lit0.is_pure());
  EXPECT_TRUE(lit1.is_pure());
}

TEST_F(IrOperatorTest, Projection) {
  EXPECT_EQ(factory().NewProjection(0), factory().NewProjection(0));
  EXPECT_EQ(factory().NewProjection(1), factory().NewProjection(1));
  EXPECT_NE(factory().NewProjection(1), factory().NewProjection(0));
}

TEST_F(IrOperatorTest, Simple) {
  EXPECT_EQ(factory().NewExit(), factory().NewExit());
  EXPECT_TRUE(factory().NewExit().is_control());

  EXPECT_EQ(factory().NewStart(), factory().NewStart());
  EXPECT_TRUE(factory().NewStart().is_control());
}

TEST_F(IrOperatorTest, Tuple) {
  EXPECT_EQ(factory().NewTuple(0), factory().NewTuple(0));
  EXPECT_EQ(factory().NewTuple(1), factory().NewTuple(1));
  EXPECT_NE(factory().NewTuple(1), factory().NewTuple(0));
  EXPECT_FALSE(factory().NewTuple(3).is_variadic());
  EXPECT_EQ(3, factory().NewTuple(3).arity());
}

}  // namespace ir
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/ir/type.h"

#include "joana/base/memory/zone.h"
#include "joana/ir/composite_types.h"
#include "joana/ir/type_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace ir {

namespace {

std::string ToString(const Type& type) {
  std::ostringstream ostream;
  ostream << type;
  return ostream.str();
}

}  // namespace

//
// IrTypeTest
//
class IrTypeTest : public ::testing::Test {
 public:
  ~IrTypeTest() override = default;

  TypeFactory& factory() { return type_factory_; }

 protected:
  IrTypeTest();

#define V(capital, underscore) \
  auto& underscore##_type() { return factory().underscore##_type(); }
  FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V

 private:
  Zone zone_;
  TypeFactory type_factory_;

  DISALLOW_COPY_AND_ASSIGN(IrTypeTest);
};

IrTypeTest::IrTypeTest() : zone_("IrTypeTest"), type_factory_(&zone_) {}

TEST_F(IrTypeTest, FunctionType) {
  auto& type1 = factory().NewFunctionType(
      factory().NewTupleType({&control_type(), &effect_type(), &int64_type()}),
      int64_type());
  EXPECT_EQ("Function<(Control, Effect, Int64) => Int64>", ToString(type1));
}

TEST_F(IrTypeTest, PrimitiveTypes) {
  EXPECT_EQ(any_type(), factory().any_type())
      << "AnyType should be a singleton.";
  EXPECT_EQ("Any", ToString(any_type()));

  EXPECT_EQ(bool_type(), factory().bool_type())
      << "BoolType should be a singleton.";
  EXPECT_NE(bool_type(), any_type());
  EXPECT_EQ("Bool", ToString(bool_type()));

  EXPECT_EQ(float64_type(), factory().float64_type())
      << "Float64Type should be a singleton.";
  EXPECT_NE(float64_type(), any_type());
  EXPECT_EQ("Float64", ToString(float64_type()));

  EXPECT_EQ(nil_type(), factory().nil_type())
      << "NilType should be a singleton.";
  EXPECT_NE(nil_type(), any_type());
  EXPECT_EQ("Nil", ToString(nil_type()));

  EXPECT_EQ(void_type(), factory().void_type())
      << "VoidType should be a singleton.";
  EXPECT_NE(void_type(), any_type());
  EXPECT_EQ("Void", ToString(void_type()));
}

TEST_F(IrTypeTest, ReferenceType) {
  auto& type1 = factory().NewReferenceType(int64_type());
  EXPECT_EQ(type1, factory().NewReferenceType(int64_type()));
  EXPECT_EQ("Reference<Int64>", ToString(type1));
}

TEST_F(IrTypeTest, TupleType) {
  auto& tuple0 = factory().NewTupleType({}).As<TupleType>();
  EXPECT_EQ(0u, tuple0.size());
  EXPECT_EQ(factory().NewTupleType({}), tuple0);
  EXPECT_EQ("Tuple<>", ToString(tuple0));

  auto& tuple1 = factory().NewTupleType({&any_type()}).As<TupleType>();
  EXPECT_EQ(1u, tuple1.size());
  EXPECT_EQ(any_type(), tuple1.get(0));
  EXPECT_EQ(factory().NewTupleType({&any_type()}), tuple1);
  EXPECT_NE(tuple0, tuple1);
  EXPECT_EQ("Tuple<Any>", ToString(tuple1));

  auto& tuple2 =
      factory().NewTupleType({&control_type(), &bool_type()}).As<TupleType>();
  EXPECT_EQ(2u, tuple2.size());
  EXPECT_EQ(control_type(), tuple2.get(0));
  EXPECT_EQ(bool_type(), tuple2.get(1));
  EXPECT_NE(tuple0, tuple2);
  EXPECT_NE(tuple1, tuple2);
  EXPECT_EQ(factory().NewTupleType({&control_type(), &bool_type()}), tuple2);
  EXPECT_EQ("Tuple<Control, Bool>", ToString(tuple2));

  auto& tuple3 =
      factory()
          .NewTupleType({&control_type(), &effect_type(), &int64_type()})
          .As<TupleType>();
  EXPECT_EQ(3u, tuple3.size());
  EXPECT_EQ(control_type(), tuple3.get(0));
  EXPECT_EQ(effect_type(), tuple3.get(1));
  EXPECT_EQ(int64_type(), tuple3.get(2));
  EXPECT_NE(tuple0, tuple3);
  EXPECT_NE(tuple1, tuple3);
  EXPECT_EQ(
      factory().NewTupleType({&control_type(), &effect_type(), &int64_type()}),
      tuple3);
  EXPECT_EQ("Tuple<Control, Effect, Int64>", ToString(tuple3));
}

TEST_F(IrTypeTest, UnionType) {
  auto& union0 = factory().NewUnionType({});
  EXPECT_EQ(factory().NewUnionType({}), union0);
  EXPECT_EQ("Nil", ToString(union0)) << "No member union type is nil type";

  auto& union1 = factory().NewUnionType({&int64_type()});
  EXPECT_EQ(factory().NewUnionType({&int64_type()}), union1);
  EXPECT_EQ(int64_type(), union1)
      << "Single member union should be its member type";
  EXPECT_EQ("Int64", ToString(union1));

  auto& union2 = factory().NewUnionType({&int64_type(), &bool_type()});
  EXPECT_EQ(union2, factory().NewUnionType({&int64_type(), &bool_type()}));
  EXPECT_EQ(union2, factory().NewUnionType({&bool_type(), &int64_type()}))
      << "Order of members does not affect identity of union type";
  EXPECT_EQ("Union<Bool, Int64>", ToString(union2));

  auto& union3 = factory().NewUnionType({&float64_type(), &union2});
  EXPECT_EQ(union3, factory().NewUnionType({&float64_type(), &union2}))
      << "Unions have same member should be identical";
  EXPECT_EQ(union3, factory().NewUnionType({&union2, &float64_type()}))
      << "Order of members does not affect identity of union type";
  EXPECT_EQ(
      factory().NewUnionType({&bool_type(), &float64_type(), &int64_type()}),
      union3)
      << "Union should be flat";
  EXPECT_EQ("Union<Bool, Float64, Int64>", ToString(union3));
}

}  // namespace ir
}  // namespace joana

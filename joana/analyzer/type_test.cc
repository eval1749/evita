// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "base/macros.h"
#include "joana/analyzer/type_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/memory/zone.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace analyzer {

class TypeTest : public ::testing::Test {
 protected:
  TypeTest();
  ~TypeTest() override;

  TypeFactory& type_factory() { return type_factory_; }

  const Type& any_type() { return type_factory_.any_type(); }
  const Type& boolean_type() {
    return GetPrimitiveType(ast::TokenKind::Boolean);
  }
  const Type& invalid_type() { return type_factory_.invalid_type(); }
  const Type& nil_type() { return type_factory_.nil_type(); }
  const Type& number_type() { return GetPrimitiveType(ast::TokenKind::Number); }
  const Type& string_type() { return GetPrimitiveType(ast::TokenKind::String); }
  const Type& unspecified_type() { return type_factory_.unspecified_type(); }
  const Type& void_type() { return type_factory_.void_type(); }

  const Type& GetPrimitiveType(ast::TokenKind kind) {
    return type_factory_.GetPrimitiveType(kind);
  }

  template <typename... Parameters>
  const Type& NewUnionType(const Parameters&... parameters) {
    return type_factory_.NewUnionTypeFromVector({&parameters...});
  }

 private:
  Zone zone_;

  // |TypeFactory| constructor takes |Zone|.
  TypeFactory type_factory_;

  DISALLOW_COPY_AND_ASSIGN(TypeTest);
};

TypeTest::TypeTest() : zone_("TypeTest"), type_factory_(&zone_) {}
TypeTest::~TypeTest() = default;

TEST_F(TypeTest, UnionType) {
  EXPECT_EQ(nil_type(), NewUnionType());
  EXPECT_EQ(nil_type(), NewUnionType(nil_type()));
  EXPECT_EQ(any_type(), NewUnionType(any_type()));
  EXPECT_EQ(any_type(), NewUnionType(any_type(), any_type()));
  EXPECT_EQ(any_type(), NewUnionType(any_type(), number_type()));
  EXPECT_EQ(NewUnionType(string_type(), number_type()),
            NewUnionType(string_type(), number_type()));
  EXPECT_EQ(NewUnionType(number_type(), string_type()),
            NewUnionType(string_type(), number_type()))
      << "order of member types does not affect identity of union type.";
  EXPECT_EQ(NewUnionType(boolean_type(), number_type(), string_type()),
            NewUnionType(NewUnionType(boolean_type(), number_type()),
                         NewUnionType(string_type(), number_type())))
      << "Nested union type should be flatten.";
}

}  // namespace analyzer
}  // namespace joana

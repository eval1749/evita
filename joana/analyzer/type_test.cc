// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/analyzer/type_factory.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code_factory.h"
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
    return NewPrimitiveType(ast::TokenKind::Boolean);
  }
  const Type& invalid_type() { return type_factory_.invalid_type(); }
  const Type& nil_type() { return type_factory_.nil_type(); }
  const Type& number_type() { return NewPrimitiveType(ast::TokenKind::Number); }
  const Type& string_type() { return NewPrimitiveType(ast::TokenKind::String); }
  const Type& unspecified_type() { return type_factory_.unspecified_type(); }
  const Type& void_type() { return type_factory_.void_type(); }

  Zone& zone() { return zone_; }

  const Type& NewPrimitiveType(ast::TokenKind kind) {
    return type_factory_.NewPrimitiveType(kind);
  }

  const Type& NewRecordType(const ast::Node& name1, const Type& type1) {
    const auto& member1 =
        type_factory_.NewLabeledType(name1, type1).As<LabeledType>();
    return type_factory_.NewRecordType({&member1});
  }

  const Type& NewRecordType(const ast::Node& name1,
                            const Type& type1,
                            const ast::Node& name2,
                            const Type& type2) {
    const auto& member1 =
        type_factory_.NewLabeledType(name1, type1).As<LabeledType>();
    const auto& member2 =
        type_factory_.NewLabeledType(name2, type2).As<LabeledType>();
    return type_factory_.NewRecordType({&member1, &member2});
  }

  template <typename... Parameters>
  const Type& NewTupleType(const Parameters&... parameters) {
    return type_factory_.NewTupleTypeFromVector({&parameters...});
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

TEST_F(TypeTest, RecordType) {
  SourceCode::Factory source_code_factory(&zone());
  const auto& text16 = base::UTF8ToUTF16("foofoobarbar");
  const auto size = text16.size() * sizeof(base::char16);
  auto* data = static_cast<base::char16*>(zone().Allocate(size));
  ::memcpy(data, text16.data(), size);
  const auto& source_code = source_code_factory.New(
      base::FilePath(), base::StringPiece16(data, text16.size()));
  ast::NodeFactory node_factory(&zone());
  const auto& foo1 = node_factory.NewName(source_code.Slice(0, 3));
  const auto& foo2 = node_factory.NewName(source_code.Slice(3, 6));
  const auto& bar1 = node_factory.NewName(source_code.Slice(6, 9));
  const auto& bar2 = node_factory.NewName(source_code.Slice(9, 12));

  EXPECT_EQ(NewRecordType(foo1, any_type()), NewRecordType(foo1, any_type()));
  EXPECT_EQ(NewRecordType(foo2, any_type()), NewRecordType(foo1, any_type()));
  EXPECT_NE(NewRecordType(foo1, any_type()),
            NewRecordType(foo1, number_type()));

  EXPECT_EQ(NewRecordType(foo1, any_type(), bar1, any_type()),
            NewRecordType(foo1, any_type(), bar1, any_type()));
  EXPECT_EQ(NewRecordType(foo1, any_type(), bar1, any_type()),
            NewRecordType(foo2, any_type(), bar2, any_type()));
  EXPECT_EQ(NewRecordType(bar1, any_type(), foo1, any_type()),
            NewRecordType(foo2, any_type(), bar2, any_type()));
}

TEST_F(TypeTest, TupleType) {
  EXPECT_EQ(NewTupleType(), NewTupleType());
  EXPECT_NE(NewTupleType(any_type()), NewTupleType());
  EXPECT_EQ(NewTupleType(any_type()), NewTupleType(any_type()));
  EXPECT_EQ(NewTupleType(any_type(), number_type()),
            NewTupleType(any_type(), number_type()));
  EXPECT_NE(NewTupleType(number_type(), any_type()),
            NewTupleType(any_type(), number_type()));
}

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

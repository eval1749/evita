// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "joana/analyzer/values.h"
#include "joana/ast/expressions.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// AnyType
//
AnyType::AnyType(int id) : Type(id) {}
AnyType::~AnyType() = default;

//
// ClassType
//
ClassType::ClassType(int id, Class* value) : Type(id), value_(*value) {
  DCHECK(value);
}

ClassType::~ClassType() = default;

const ast::Node& ClassType::name() const {
  return value_.node().child_at(0);
}

//
// GenericType
//
GenericType::GenericType(Zone* zone,
                         int id,
                         const std::vector<const TypeParameter*>& parameters)
    : Type(id), parameters_(zone, parameters) {}

GenericType::~GenericType() = default;

//
// FunctionType
//
FunctionType::FunctionType(Zone* zone,
                           int id,
                           FunctionTypeKind kind,
                           const std::vector<const TypeParameter*>& parameters,
                           std::vector<const Type*> parameter_types,
                           const Type& return_type,
                           const Type& this_type)
    : GenericType(zone, id, parameters),
      kind_(kind),
      parameter_types_(zone, parameter_types),
      return_type_(return_type),
      this_type_(this_type) {}

FunctionType::~FunctionType() = default;

//
// InvalidType
//
InvalidType::InvalidType(int id) : Type(id) {}
InvalidType::~InvalidType() = default;

//
// NamedType
//
NamedType::NamedType(int id, const ast::Node& name) : Type(id), name_(name) {
  DCHECK(name.Is<ast::Empty>() || name.Is<ast::Name>() ||
         name.Is<ast::MemberExpression>() ||
         name.Is<ast::ComputedMemberExpression>());
}

NamedType::~NamedType() = default;

bool NamedType::is_anonymous() const {
  return name_.Is<ast::Empty>();
}

//
// NilType
//
NilType::NilType(int id) : Type(id) {}
NilType::~NilType() = default;

//
// NullType
//
NullType::NullType(int id) : Type(id) {}
NullType::~NullType() = default;

//
// PrimitiveType
//
PrimitiveType::PrimitiveType(int id, const ast::Node& name)
    : NamedType(id, name) {}
PrimitiveType::~PrimitiveType() = default;

//
// TypeApplication
//
TypeApplication::TypeApplication(Zone* zone,
                                 int id,
                                 const GenericType& generic_type,
                                 const std::vector<Argument>& arguments)
    : Type(id), arguments_(zone, arguments), generic_type_(generic_type) {}

TypeApplication::~TypeApplication() = default;

//
// TypeName
//
TypeName::TypeName(int id, const ast::Node& name) : NamedType(id, name) {}
TypeName::~TypeName() = default;

//
// TypeParameter
//
TypeParameter::TypeParameter(int id, const ast::Node& name)
    : NamedType(id, name) {}
TypeParameter::~TypeParameter() = default;

//
// UnionType
//
UnionType::UnionType(int id, const std::vector<const Type*>& members)
    : Type(id), number_of_members_(members.size()) {
  DCHECK_GE(number_of_members_, 2);
  auto* runner = members_;
  for (const auto& member : members) {
    *runner = member;
    runner++;
  }
}

UnionType::~UnionType() = default;

BlockRange<const Type*> UnionType::members() const {
  return BlockRange<const Type*>(&members_[0], number_of_members_);
}

//
// UnspecifiedType
//
UnspecifiedType::UnspecifiedType(int id) : Type(id) {}
UnspecifiedType::~UnspecifiedType() = default;

//
// VoidType
//
VoidType::VoidType(int id) : Type(id) {}
VoidType::~VoidType() = default;

}  // namespace analyzer
}  // namespace joana

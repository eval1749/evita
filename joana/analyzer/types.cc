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
FunctionType::FunctionType(
    int id,
    FunctionTypeKind kind,
    const std::vector<const TypeParameter*>& type_parameters,
    const std::vector<const Type*>& parameters,
    const Type& return_type,
    const Type& this_type)
    : Type(id),
      kind_(kind),
      number_of_parameters_(parameters.size()),
      number_of_type_parameters_(type_parameters.size()),
      return_type_(return_type),
      this_type_(this_type) {
  auto* runner = elements_;
  for (const auto& parameter : parameters) {
    *runner = parameter;
    ++runner;
  }
  for (const auto& type_parameter : type_parameters) {
    *runner = type_parameter;
    ++runner;
  }
}

FunctionType::~FunctionType() = default;

BlockRange<const Type*> FunctionType::parameters() const {
  return BlockRange<const Type*>(elements_, number_of_parameters_);
}

BlockRange<const TypeParameter*> FunctionType::type_parameters() const {
  return BlockRange<const TypeParameter*>(
      reinterpret_cast<const TypeParameter* const*>(
          &elements_[number_of_parameters_]),
      number_of_type_parameters_);
}

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
// TupleType
//
TupleType::TupleType(int id, const std::vector<const Type*>& members)
    : Type(id), number_of_members_(members.size()) {
  auto* runner = members_;
  for (const auto& member : members) {
    *runner = member;
    runner++;
  }
}

TupleType::~TupleType() = default;

BlockRange<const Type*> TupleType::members() const {
  return BlockRange<const Type*>(&members_[0], number_of_members_);
}

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
UnionType::UnionType(int id, const std::set<const Type*>& members)
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

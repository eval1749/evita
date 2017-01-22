// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "joana/analyzer/values.h"
#include "joana/ast/expressions.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

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
ClassType::ClassType(int id, Value* value) : Type(id), value_(*value) {
  DCHECK(value->Is<Class>() || value->Is<ConstructedClass>()) << value;
}

ClassType::~ClassType() = default;

const ast::Node& ClassType::name() const {
  return value_.node().child_at(0);
}

//
// FunctionType
//
FunctionType::FunctionType(
    int id,
    FunctionTypeKind kind,
    const std::vector<const TypeParameter*>& type_parameters,
    const FunctionTypeArity& arity,
    const std::vector<const Type*>& parameters,
    const Type& return_type,
    const Type& this_type)
    : Type(id),
      arity_(arity),
      kind_(kind),
      number_of_parameters_(parameters.size()),
      number_of_type_parameters_(type_parameters.size()),
      return_type_(return_type),
      this_type_(this_type) {
  DCHECK_GE(arity_.minimum, 0);
  DCHECK_GE(arity_.maximum, arity_.minimum);
  if (arity_.has_rest)
    DCHECK_EQ(static_cast<size_t>(arity_.maximum) + 1, parameters.size());
  else
    DCHECK_EQ(static_cast<size_t>(arity_.maximum), parameters.size());
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
// FunctionTypeArity
//
bool operator<(const FunctionTypeArity& arity1,
               const FunctionTypeArity& arity2) {
  if (arity1.minimum != arity2.minimum)
    return arity1.minimum < arity2.minimum;
  if (arity1.maximum != arity2.maximum)
    return arity1.maximum < arity2.maximum;
  if (arity1.has_rest != arity2.has_rest)
    return arity2.has_rest;
  return false;
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
// LabeledType
//
LabeledType::LabeledType(int id, const ast::Node& name, const Type& type)
    : Type(id), name_(name), type_(type) {}

LabeledType::~LabeledType() = default;

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
// RecordType
//
RecordType::RecordType(int id, const std::vector<const LabeledType*>& members)
    : Type(id), number_of_members_(members.size()) {
  auto* runner = members_;
  for (const auto& member : members) {
    *runner = member;
    runner++;
  }
}

RecordType::~RecordType() = default;

BlockRange<const LabeledType*> RecordType::members() const {
  return BlockRange<const LabeledType*>(&members_[0], number_of_members_);
}

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
// TypeAlias
//
TypeAlias::TypeAlias(int id, const ast::Node& name, const ast::Node& type)
    : NamedType(id, name), type_(type) {
  DCHECK(type.syntax().Is<ast::Type>()) << name << ' ' << type;
}

TypeAlias::~TypeAlias() = default;

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

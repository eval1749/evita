// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/types.h"

namespace joana {
namespace ast {

//
// AnyType
//
AnyType::AnyType() : Type(SyntaxCode::AnyType, Format()) {}
AnyType::~AnyType() = default;

//
// FunctionType
//
FunctionType::FunctionType(FunctionTypeKind kind)
    : Type(SyntaxCode::FunctionType,
           Format::Builder().set_arity(2).set_number_of_parameters(1).Build()),
      kind_(kind) {}

FunctionType::~FunctionType() = default;

//
// InvalidType
//
InvalidType::InvalidType() : Type(SyntaxCode::InvalidType, Format()) {}
InvalidType::~InvalidType() = default;

//
// MemberType
//
MemberType::MemberType()
    : Type(SyntaxCode::MemberType, Format::Builder().set_arity(2).Build()) {}

MemberType::~MemberType() = default;

const Node& MemberType::MemberOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::MemberType);
  return node.child_at(0);
}

const Node& MemberType::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::MemberType);
  return node.child_at(1);
}

//
// NullableType
//
NullableType::NullableType()
    : Type(SyntaxCode::NullableType, Format::Builder().set_arity(1).Build()) {}

NullableType::~NullableType() = default;

const Node& NullableType::TypeOf(const Node& node) {
  return node.child_at(0);
}

//
// NonNullableType
//
NonNullableType::NonNullableType()
    : Type(SyntaxCode::NonNullableType,
           Format::Builder().set_arity(1).Build()) {}
NonNullableType::~NonNullableType() = default;

const Node& NonNullableType::TypeOf(const Node& node) {
  return node.child_at(0);
}

//
// OptionalType
//
OptionalType::OptionalType()
    : Type(SyntaxCode::OptionalType, Format::Builder().set_arity(1).Build()) {}
OptionalType::~OptionalType() = default;

const Node& OptionalType::TypeOf(const Node& node) {
  return node.child_at(0);
}

//
// PrimitiveType
//
PrimitiveType::PrimitiveType()
    : Type(SyntaxCode::PrimitiveType, Format::Builder().set_arity(1).Build()) {}

PrimitiveType::~PrimitiveType() = default;

const Node& PrimitiveType::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::PrimitiveType);
  return node.child_at(0);
}

//
// RecordType
//
RecordType::RecordType()
    : Type(SyntaxCode::RecordType,
           Format::Builder().set_is_variadic(true).Build()) {}

RecordType::~RecordType() = default;

//
// RestType
//
RestType::RestType()
    : Type(SyntaxCode::RestType, Format::Builder().set_arity(1).Build()) {}
RestType::~RestType() = default;

const Node& RestType::TypeOf(const Node& node) {
  return node.child_at(0);
}

//
// TupleType
//
TupleType::TupleType()
    : Type(SyntaxCode::TupleType,
           Format::Builder().set_is_variadic(true).Build()) {}

TupleType::~TupleType() = default;

//
// Type
//
Type::Type(SyntaxCode syntax_code, const Format& format)
    : Syntax(syntax_code, format) {}

Type::~Type() = default;

//
// TypeApplication
//
TypeApplication::TypeApplication()
    : Type(SyntaxCode::TypeApplication,
           Format::Builder().set_arity(2).Build()) {}

TypeApplication::~TypeApplication() = default;

const Node& TypeApplication::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeApplication);
  return node.child_at(1);
}

const Node& TypeApplication::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeApplication);
  return node.child_at(0);
}

//
// TypeGroup
//
TypeGroup::TypeGroup()
    : Type(SyntaxCode::TypeGroup, Format::Builder().set_arity(1).Build()) {}
TypeGroup::~TypeGroup() = default;

const Node& TypeGroup::TypeOf(const Node& node) {
  return node.child_at(0);
}

//
// TypeName
//
TypeName::TypeName()
    : Type(SyntaxCode::TypeName, Format::Builder().set_arity(1).Build()) {}

TypeName::~TypeName() = default;

const Node& TypeName::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeName);
  return node.child_at(0);
}

//
// UnionType
//
UnionType::UnionType()
    : Type(SyntaxCode::UnionType,
           Format::Builder().set_is_variadic(true).Build()) {}

UnionType::~UnionType() = default;

//
// UnknownType
//
UnknownType::UnknownType() : Type(SyntaxCode::UnknownType, Format()) {}
UnknownType::~UnknownType() = default;

//
// VoidType
//
VoidType::VoidType() : Type(SyntaxCode::VoidType, Format()) {}
VoidType::~VoidType() = default;

}  // namespace ast
}  // namespace joana

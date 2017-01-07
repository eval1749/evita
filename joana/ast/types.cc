// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/types.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Type, AnyType, 0)
IMPLEMENT_AST_SYNTAX_1(Type, FunctionType, 2, FunctionTypeKind, kind)
IMPLEMENT_AST_SYNTAX_0(Type, InvalidType, 0)
IMPLEMENT_AST_SYNTAX_0(Type, NullableType, 1)
IMPLEMENT_AST_SYNTAX_0(Type, NonNullableType, 1)
IMPLEMENT_AST_SYNTAX_0(Type, OptionalType, 1)
IMPLEMENT_AST_SYNTAX_0(Type, RestType, 1)
IMPLEMENT_AST_SYNTAX_0(Type, TypeGroup, 1)
IMPLEMENT_AST_SYNTAX_0(Type, UnknownType, 0)
IMPLEMENT_AST_SYNTAX_0(Type, VoidType, 0)

//
// RecordType
//
RecordType::RecordType()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RecordType,
                     Format::Builder().set_is_variadic(true).Build()) {}

RecordType::~RecordType() = default;

//
// TupleType
//
TupleType::TupleType()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::TupleType,
                     Format::Builder().set_is_variadic(true).Build()) {}

TupleType::~TupleType() = default;

//
// TypeApplication
//
TypeApplication::TypeApplication()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::TypeApplication,
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
// TypeName
//
TypeName::TypeName()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::TypeName,
                     Format::Builder().set_arity(1).Build()) {}

TypeName::~TypeName() = default;

const Node& TypeName::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeName);
  return node.child_at(0);
}

//
// UnionType
//
UnionType::UnionType()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::UnionType,
                     Format::Builder().set_is_variadic(true).Build()) {}

UnionType::~UnionType() = default;

}  // namespace ast
}  // namespace joana

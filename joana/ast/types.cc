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
IMPLEMENT_AST_SYNTAX_0(Type, TypeName, 1)
IMPLEMENT_AST_SYNTAX_0(Type, TypeProperty, 2)
IMPLEMENT_AST_SYNTAX_0(Type, UnknownType, 0)
IMPLEMENT_AST_SYNTAX_0(Type, VoidType, 0)

//
// RecordTypeSyntax
//
RecordTypeSyntax::RecordTypeSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RecordType,
                     Format::Builder().set_is_variadic(true).Build()) {}

RecordTypeSyntax::~RecordTypeSyntax() = default;

//
// TupleTypeSyntax
//
TupleTypeSyntax::TupleTypeSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::TupleType,
                     Format::Builder().set_is_variadic(true).Build()) {}

TupleTypeSyntax::~TupleTypeSyntax() = default;

//
// TypeApplicationSyntax
//
TypeApplicationSyntax::TypeApplicationSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::TypeApplication,
                     Format::Builder().set_arity(2).Build()) {}

TypeApplicationSyntax::~TypeApplicationSyntax() = default;

const Node& TypeApplicationSyntax::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeApplication);
  return node.child_at(1);
}

const Node& TypeApplicationSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::TypeApplication);
  return node.child_at(0);
}

//
// UnionTypeSyntax
//
UnionTypeSyntax::UnionTypeSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::UnionType,
                     Format::Builder().set_is_variadic(true).Build()) {}

UnionTypeSyntax::~UnionTypeSyntax() = default;

}  // namespace ast
}  // namespace joana

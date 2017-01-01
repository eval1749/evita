// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/expressions.h"

namespace joana {
namespace ast {

//
// ArgumentListSyntax
//
ArgumentListSyntax::ArgumentListSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ArgumentList,
                     Format::Builder().set_is_variadic(true).Build()) {}

ArgumentListSyntax::~ArgumentListSyntax() = default;

//
// ArrayInitializerSyntax
//
ArrayInitializerSyntax::ArrayInitializerSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ArgumentList,
                     Format::Builder().set_is_variadic(true).Build()) {}

ArrayInitializerSyntax::~ArrayInitializerSyntax() = default;

//
// AssignmentExpressionSyntax
//
AssignmentExpressionSyntax::AssignmentExpressionSyntax(PunctuatorKind kind)
    : SyntaxTemplate(kind,
                     SyntaxCode::AssignmentExpression,
                     Format::Builder().set_arity(3).Build()) {}

AssignmentExpressionSyntax::~AssignmentExpressionSyntax() = default;

const Node& AssignmentExpressionSyntax::LeftHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(0);
}

const Node& AssignmentExpressionSyntax::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(1);
}

const Node& AssignmentExpressionSyntax::RightHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(2);
}

//
// BinaryExpressionSyntax
//
BinaryExpressionSyntax::BinaryExpressionSyntax(PunctuatorKind kind)
    : SyntaxTemplate(kind,
                     SyntaxCode::BinaryExpression,
                     Format::Builder().set_arity(3).Build()) {}

BinaryExpressionSyntax::~BinaryExpressionSyntax() = default;

const Node& BinaryExpressionSyntax::LeftHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(0);
}

const Node& BinaryExpressionSyntax::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(1);
}

const Node& BinaryExpressionSyntax::RightHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(2);
}

IMPLEMENT_AST_SYNTAX_0(Expression, CallExpression, 2)

//
// CommaExpressionSyntax
//
CommaExpressionSyntax::CommaExpressionSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::CommaExpression,
                     Format::Builder().set_is_variadic(true).Build()) {}

CommaExpressionSyntax::~CommaExpressionSyntax() = default;

IMPLEMENT_AST_SYNTAX_0(Expression, ComputedMemberExpression, 2)
IMPLEMENT_AST_SYNTAX_0(Expression, ConditionalExpression, 1)
IMPLEMENT_AST_SYNTAX_0(Expression, DelimiterExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, GroupExpression, 1)
IMPLEMENT_AST_SYNTAX_0(Expression, ElisionExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, NewExpression, 2)
IMPLEMENT_AST_SYNTAX_0(Expression, MemberExpression, 2)

//
// ObjectInitializerSyntax
//
ObjectInitializerSyntax::ObjectInitializerSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ArgumentList,
                     Format::Builder().set_is_variadic(true).Build()) {}

ObjectInitializerSyntax::~ObjectInitializerSyntax() = default;

//
// ParameterListSyntax
//
ParameterListSyntax::ParameterListSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ParameterList,
                     Format::Builder().set_is_variadic(true).Build()) {}

ParameterListSyntax::~ParameterListSyntax() = default;

//
// PropertySyntax
//
PropertySyntax::PropertySyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Property,
                     Format::Builder().set_arity(2).Build()) {}

PropertySyntax::~PropertySyntax() = default;

const Node& PropertySyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Property);
  return node.child_at(0);
}

const Node& PropertySyntax::ValueOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Property);
  return node.child_at(1);
}

//
// ReferenceExpressionSyntax
//
ReferenceExpressionSyntax::ReferenceExpressionSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ReferenceExpression,
                     Format::Builder().set_arity(1).Build()) {}

ReferenceExpressionSyntax::~ReferenceExpressionSyntax() = default;

const Node& ReferenceExpressionSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ReferenceExpression);
  return node.child_at(0);
}

//
// RegExpLiteralExpressionSyntax
//
RegExpLiteralExpressionSyntax::RegExpLiteralExpressionSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RegExpLiteralExpression,
                     Format::Builder().set_arity(2).Build()) {}

RegExpLiteralExpressionSyntax::~RegExpLiteralExpressionSyntax() = default;

const Node& RegExpLiteralExpressionSyntax::RegExpOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::RegExpLiteralExpression);
  return node.child_at(0);
}

const Node& RegExpLiteralExpressionSyntax::FlagsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::RegExpLiteralExpression);
  return node.child_at(1);
}

//
// TupleSyntax
//
TupleSyntax::TupleSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Tuple,
                     Format::Builder().set_is_variadic(true).Build()) {}

TupleSyntax::~TupleSyntax() = default;

//
// UnaryExpressionSyntax
//
UnaryExpressionSyntax::UnaryExpressionSyntax(PunctuatorKind kind)
    : SyntaxTemplate(kind,
                     SyntaxCode::UnaryExpression,
                     Format::Builder().set_arity(2).Build()) {}

UnaryExpressionSyntax::~UnaryExpressionSyntax() = default;

const Node& UnaryExpressionSyntax::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::UnaryExpression);
  return node.child_at(1);
}

const Node& UnaryExpressionSyntax::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::UnaryExpression);
  return node.child_at(0);
}

}  // namespace ast
}  // namespace joana

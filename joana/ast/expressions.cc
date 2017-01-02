// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/expressions.h"

#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Expression, ComputedMemberExpression, 2)
IMPLEMENT_AST_SYNTAX_0(Expression, ConditionalExpression, 3)
IMPLEMENT_AST_SYNTAX_0(Expression, DelimiterExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, ElisionExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, GroupExpression, 1)
IMPLEMENT_AST_SYNTAX_0(Expression, MemberExpression, 2)

//
// ArrayInitializerSyntax
//
ArrayInitializerSyntax::ArrayInitializerSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ArrayInitializer,
                     Format::Builder().set_is_variadic(true).Build()) {}

ArrayInitializerSyntax::~ArrayInitializerSyntax() = default;

//
// AssignmentExpressionSyntax
//
AssignmentExpressionSyntax::AssignmentExpressionSyntax(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::AssignmentExpression,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

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
BinaryExpressionSyntax::BinaryExpressionSyntax(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::BinaryExpression,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

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

//
// CallExpressionSyntax
//
CallExpressionSyntax::CallExpressionSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::CallExpression,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

CallExpressionSyntax::~CallExpressionSyntax() = default;

ChildNodes CallExpressionSyntax::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CallExpression);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& CallExpressionSyntax::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CallExpression);
  return node.child_at(0);
}

//
// CommaExpressionSyntax
//
CommaExpressionSyntax::CommaExpressionSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::CommaExpression,
                     Format::Builder().set_is_variadic(true).Build()) {}

CommaExpressionSyntax::~CommaExpressionSyntax() = default;

//
// NewExpressionSyntax
//
NewExpressionSyntax::NewExpressionSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::NewExpression,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

NewExpressionSyntax::~NewExpressionSyntax() = default;

ChildNodes NewExpressionSyntax::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::NewExpression);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& NewExpressionSyntax::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::NewExpression);
  return node.child_at(0);
}

//
// ObjectInitializerSyntax
//
ObjectInitializerSyntax::ObjectInitializerSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ObjectInitializer,
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
UnaryExpressionSyntax::UnaryExpressionSyntax(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::UnaryExpression,
          Format::Builder().set_arity(2).set_number_of_parameters(1).Build()) {}

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

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/expressions.h"

#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Expression, ConditionalExpression, 3)
IMPLEMENT_AST_SYNTAX_0(Expression, DelimiterExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, ElisionExpression, 0)
IMPLEMENT_AST_SYNTAX_0(Expression, GroupExpression, 1)

//
// ArrayInitializer
//
ArrayInitializer::ArrayInitializer()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ArrayInitializer,
                     Format::Builder().set_is_variadic(true).Build()) {}

ArrayInitializer::~ArrayInitializer() = default;

//
// AssignmentExpression
//
AssignmentExpression::AssignmentExpression(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::AssignmentExpression,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

AssignmentExpression::~AssignmentExpression() = default;

const Node& AssignmentExpression::LeftHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(0);
}

const Node& AssignmentExpression::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(1);
}

const Node& AssignmentExpression::RightHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::AssignmentExpression);
  return node.child_at(2);
}

//
// BinaryExpression
//
BinaryExpression::BinaryExpression(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::BinaryExpression,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

BinaryExpression::~BinaryExpression() = default;

const Node& BinaryExpression::LeftHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(0);
}

const Node& BinaryExpression::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(1);
}

const Node& BinaryExpression::RightHandSideOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BinaryExpression);
  return node.child_at(2);
}

//
// CallExpression
//
CallExpression::CallExpression()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::CallExpression,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

CallExpression::~CallExpression() = default;

ChildNodes CallExpression::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CallExpression);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& CallExpression::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CallExpression);
  return node.child_at(0);
}

//
// CommaExpression
//
CommaExpression::CommaExpression()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::CommaExpression,
                     Format::Builder().set_is_variadic(true).Build()) {}

CommaExpression::~CommaExpression() = default;

//
// ComputedMemberExpression
//
ComputedMemberExpression::ComputedMemberExpression()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ComputedMemberExpression,
                     Format::Builder().set_arity(2).Build()) {}

ComputedMemberExpression::~ComputedMemberExpression() = default;

const Node& ComputedMemberExpression::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ComputedMemberExpression);
  return node.child_at(1);
}

const Node& ComputedMemberExpression::MemberExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ComputedMemberExpression);
  return node.child_at(0);
}

//
// MemberExpression
//
MemberExpression::MemberExpression()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::MemberExpression,
                     Format::Builder().set_arity(2).Build()) {}

MemberExpression::~MemberExpression() = default;

const Node& MemberExpression::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::MemberExpression);
  return node.child_at(0);
}

const Node& MemberExpression::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::MemberExpression);
  return node.child_at(1);
}

//
// NewExpression
//
NewExpression::NewExpression()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::NewExpression,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

NewExpression::~NewExpression() = default;

ChildNodes NewExpression::ArgumentsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::NewExpression);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& NewExpression::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::NewExpression);
  return node.child_at(0);
}

//
// ObjectInitializer
//
ObjectInitializer::ObjectInitializer()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ObjectInitializer,
                     Format::Builder().set_is_variadic(true).Build()) {}

ObjectInitializer::~ObjectInitializer() = default;

//
// ParameterList
//
ParameterList::ParameterList()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ParameterList,
                     Format::Builder().set_is_variadic(true).Build()) {}

ParameterList::~ParameterList() = default;

//
// Property
//
Property::Property()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Property,
                     Format::Builder().set_arity(2).Build()) {}

Property::~Property() = default;

const Node& Property::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Property);
  return node.child_at(0);
}

const Node& Property::ValueOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Property);
  return node.child_at(1);
}

//
// ReferenceExpression
//
ReferenceExpression::ReferenceExpression()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ReferenceExpression,
                     Format::Builder().set_arity(1).Build()) {}

ReferenceExpression::~ReferenceExpression() = default;

const Node& ReferenceExpression::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ReferenceExpression);
  return node.child_at(0);
}

//
// RegExpLiteralExpression
//
RegExpLiteralExpression::RegExpLiteralExpression()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RegExpLiteralExpression,
                     Format::Builder().set_arity(2).Build()) {}

RegExpLiteralExpression::~RegExpLiteralExpression() = default;

const Node& RegExpLiteralExpression::RegExpOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::RegExpLiteralExpression);
  return node.child_at(0);
}

const Node& RegExpLiteralExpression::FlagsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::RegExpLiteralExpression);
  return node.child_at(1);
}

//
// Tuple
//
Tuple::Tuple()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Tuple,
                     Format::Builder().set_is_variadic(true).Build()) {}

Tuple::~Tuple() = default;

//
// UnaryExpression
//
UnaryExpression::UnaryExpression(TokenKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::UnaryExpression,
          Format::Builder().set_arity(2).set_number_of_parameters(1).Build()) {}

UnaryExpression::~UnaryExpression() = default;

const Node& UnaryExpression::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::UnaryExpression);
  return node.child_at(1);
}

const Node& UnaryExpression::OperatorOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::UnaryExpression);
  return node.child_at(0);
}

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/expressions.h"

#include "joana/ast/declarations.h"
#include "joana/ast/literals.h"
#include "joana/ast/node_editor.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace ast {

//
// ArrayLiteralExpression
//
ArrayLiteralExpression::ArrayLiteralExpression(const SourceCodeRange& range,
                                               const ExpressionList& elements)
    : NodeTemplate(&elements, range) {}

ArrayLiteralExpression::~ArrayLiteralExpression() = default;

//
// AssignmentExpression
//
AssignmentExpression::AssignmentExpression(const SourceCodeRange& range,
                                           const Punctuator& op,
                                           const Expression& lhs,
                                           const Expression& rhs)
    : NodeTemplate(std::make_tuple(&op, &lhs, &rhs), range) {}

AssignmentExpression::~AssignmentExpression() = default;

//
// BinaryExpression
//
BinaryExpression::BinaryExpression(const SourceCodeRange& range,
                                   Token* op,
                                   Expression* lhs,
                                   Expression* rhs)
    : NodeTemplate(std::make_tuple(op, lhs, rhs), range) {}

BinaryExpression::~BinaryExpression() = default;

//
// CallExpression
//
CallExpression::CallExpression(const SourceCodeRange& range,
                               Expression* callee,
                               ExpressionList* arguments)
    : NodeTemplate(std::make_tuple(callee, arguments), range) {}

CallExpression::~CallExpression() = default;

//
// CommaExpression
//
CommaExpression::CommaExpression(const SourceCodeRange& range,
                                 ExpressionList* expressions)
    : NodeTemplate(expressions, range) {}

CommaExpression::~CommaExpression() = default;

//
// ComputedMemberExpression
//
ComputedMemberExpression::ComputedMemberExpression(const SourceCodeRange& range,
                                                   Expression* expression,
                                                   Expression* name_expression)
    : NodeTemplate(std::make_tuple(expression, name_expression), range) {}

ComputedMemberExpression::~ComputedMemberExpression() = default;

//
// ConditionalExpression
//
ConditionalExpression::ConditionalExpression(const SourceCodeRange& range,
                                             Expression* condition,
                                             Expression* true_expression,
                                             Expression* false_expression)
    : NodeTemplate(
          std::make_tuple(condition, true_expression, false_expression),
          range) {}

ConditionalExpression::~ConditionalExpression() = default;

//
// DeclarationExpression
//
DeclarationExpression::DeclarationExpression(Declaration* declaration)
    : NodeTemplate(declaration, declaration->range()) {}

DeclarationExpression::~DeclarationExpression() = default;

//
// ElisionExpression
//
ElisionExpression::ElisionExpression(const SourceCodeRange& range)
    : Expression(range) {}

ElisionExpression::~ElisionExpression() = default;

//
// EmptyExpression
//
EmptyExpression::EmptyExpression(const SourceCodeRange& range)
    : Expression(range) {}

EmptyExpression::~EmptyExpression() = default;

//
// Expression
//
Expression::Expression(const SourceCodeRange& range) : ContainerNode(range) {}

Expression::~Expression() = default;

//
// ExpressionList
//
ExpressionList::ExpressionList(Zone* zone,
                               const std::vector<Expression*>& expressions)
    : expressions_(zone, expressions) {}

ExpressionList::~ExpressionList() = default;

//
// GroupExpression
//
GroupExpression::GroupExpression(const SourceCodeRange& range,
                                 Expression* expression)
    : NodeTemplate(expression, range) {}

GroupExpression::~GroupExpression() = default;

//
// InvalidExpression
//
InvalidExpression::InvalidExpression(const Node& node, int error_code)
    : Expression(node.range()), error_code_(error_code) {}

InvalidExpression::~InvalidExpression() = default;

//
// LiteralExpression
//
LiteralExpression::LiteralExpression(Literal* literal)
    : NodeTemplate(std::make_tuple(literal), literal->range()) {}

LiteralExpression::~LiteralExpression() = default;

//
// NewExpression
//
NewExpression::NewExpression(const SourceCodeRange& range,
                             Expression* expression,
                             ExpressionList* arguments)
    : NodeTemplate(std::make_tuple(expression, arguments), range) {}

NewExpression::~NewExpression() = default;

//
// ObjectLiteralExpression
//
ObjectLiteralExpression::ObjectLiteralExpression(const SourceCodeRange& range,
                                                 ExpressionList* members)
    : NodeTemplate(members, range) {}

ObjectLiteralExpression::~ObjectLiteralExpression() = default;

//
// MemberExpression
//
MemberExpression::MemberExpression(const SourceCodeRange& range,
                                   Expression* expression,
                                   Name* name)
    : NodeTemplate(std::make_tuple(expression, name), range) {}

MemberExpression::~MemberExpression() = default;

//
// PropertyDefinitionExpression
//
PropertyDefinitionExpression::PropertyDefinitionExpression(
    const SourceCodeRange& range,
    Expression* name,
    Expression* value)
    : NodeTemplate(std::make_tuple(name, value), range) {}

PropertyDefinitionExpression::~PropertyDefinitionExpression() = default;

//
// ReferenceExpression
//
ReferenceExpression::ReferenceExpression(Name* name)
    : NodeTemplate(name, name->range()) {}

ReferenceExpression::~ReferenceExpression() = default;

//
// RegExpLiteralExpression
//
RegExpLiteralExpression::RegExpLiteralExpression(const SourceCodeRange& range,
                                                 RegExp* pattern,
                                                 Token* flags)
    : NodeTemplate(std::make_tuple(pattern, flags), range) {}

RegExpLiteralExpression::~RegExpLiteralExpression() = default;

//
// UnaryExpression
//
UnaryExpression::UnaryExpression(const SourceCodeRange& range,
                                 Token* op,
                                 Expression* expression)
    : NodeTemplate(std::make_tuple(op, expression), range) {}

UnaryExpression::~UnaryExpression() = default;

}  // namespace ast
}  // namespace joana

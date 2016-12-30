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
                                   const Token& op,
                                   const Expression& lhs,
                                   const Expression& rhs)
    : NodeTemplate(std::make_tuple(&op, &lhs, &rhs), range) {}

BinaryExpression::~BinaryExpression() = default;

//
// CallExpression
//
CallExpression::CallExpression(const SourceCodeRange& range,
                               const Expression& callee,
                               const ExpressionList& arguments)
    : NodeTemplate(std::make_tuple(&callee, &arguments), range) {}

CallExpression::~CallExpression() = default;

//
// CommaExpression
//
CommaExpression::CommaExpression(const SourceCodeRange& range,
                                 const ExpressionList& expressions)
    : NodeTemplate(&expressions, range) {}

CommaExpression::~CommaExpression() = default;

//
// ComputedMemberExpression
//
ComputedMemberExpression::ComputedMemberExpression(
    const SourceCodeRange& range,
    const Expression& expression,
    const Expression& name_expression)
    : NodeTemplate(std::make_tuple(&expression, &name_expression), range) {}

ComputedMemberExpression::~ComputedMemberExpression() = default;

//
// ConditionalExpression
//
ConditionalExpression::ConditionalExpression(const SourceCodeRange& range,
                                             const Expression& condition,
                                             const Expression& true_expression,
                                             const Expression& false_expression)
    : NodeTemplate(
          std::make_tuple(&condition, &true_expression, &false_expression),
          range) {}

ConditionalExpression::~ConditionalExpression() = default;

//
// DeclarationExpression
//
DeclarationExpression::DeclarationExpression(const Declaration& declaration)
    : NodeTemplate(&declaration, declaration.range()) {}

DeclarationExpression::~DeclarationExpression() = default;

//
// DelimiterExpression
//
DelimiterExpression::DelimiterExpression(const SourceCodeRange& range)
    : Expression(range) {}

DelimiterExpression::~DelimiterExpression() = default;

//
// ElisionExpression
//
ElisionExpression::ElisionExpression(const SourceCodeRange& range)
    : Expression(range) {}

ElisionExpression::~ElisionExpression() = default;

//
// Expression
//
Expression::Expression(const SourceCodeRange& range) : Node(range) {}

Expression::~Expression() = default;

//
// ExpressionList
//
ExpressionList::ExpressionList(
    Zone* zone,
    const std::vector<const Expression*>& expressions)
    : expressions_(zone, expressions) {}

ExpressionList::~ExpressionList() = default;

//
// GroupExpression
//
GroupExpression::GroupExpression(const SourceCodeRange& range,
                                 const Expression& expression)
    : NodeTemplate(&expression, range) {}

GroupExpression::~GroupExpression() = default;

//
// InvalidExpression
//
InvalidExpression::InvalidExpression(const SourceCodeRange& range,
                                     int error_code)
    : Expression(range), error_code_(error_code) {}

InvalidExpression::~InvalidExpression() = default;

//
// LiteralExpression
//
LiteralExpression::LiteralExpression(const Literal& literal)
    : NodeTemplate(std::make_tuple(&literal), literal.range()) {}

LiteralExpression::~LiteralExpression() = default;

//
// NewExpression
//
NewExpression::NewExpression(const SourceCodeRange& range,
                             const Expression& expression,
                             const ExpressionList& arguments)
    : NodeTemplate(std::make_tuple(&expression, &arguments), range) {}

NewExpression::~NewExpression() = default;

//
// ObjectLiteralExpression
//
ObjectLiteralExpression::ObjectLiteralExpression(const SourceCodeRange& range,
                                                 const ExpressionList& members)
    : NodeTemplate(&members, range) {}

ObjectLiteralExpression::~ObjectLiteralExpression() = default;

//
// MemberExpression
//
MemberExpression::MemberExpression(const SourceCodeRange& range,
                                   const Expression& expression,
                                   const Name& name)
    : NodeTemplate(std::make_tuple(&expression, &name), range) {}

MemberExpression::~MemberExpression() = default;

//
// ParameterList
//
ParameterList::ParameterList(
    const SourceCodeRange& range,
    const std::vector<const BindingElement*>& parameters)
    : Expression(range), parameters_(parameters) {}

ParameterList::~ParameterList() = default;

//
// PropertyDefinitionExpression
//
PropertyDefinitionExpression::PropertyDefinitionExpression(
    const SourceCodeRange& range,
    const Expression& name,
    const Expression& value)
    : NodeTemplate(std::make_tuple(&name, &value), range) {}

PropertyDefinitionExpression::~PropertyDefinitionExpression() = default;

//
// ReferenceExpression
//
ReferenceExpression::ReferenceExpression(const Name& name)
    : NodeTemplate(&name, name.range()) {}

ReferenceExpression::~ReferenceExpression() = default;

//
// RegExpLiteralExpression
//
RegExpLiteralExpression::RegExpLiteralExpression(const SourceCodeRange& range,
                                                 const RegExp& pattern,
                                                 const Token& flags)
    : NodeTemplate(std::make_tuple(&pattern, &flags), range) {}

RegExpLiteralExpression::~RegExpLiteralExpression() = default;

//
// UnaryExpression
//
UnaryExpression::UnaryExpression(const SourceCodeRange& range,
                                 const Token& op,
                                 const Expression& expression)
    : NodeTemplate(std::make_tuple(&op, &expression), range) {}

UnaryExpression::~UnaryExpression() = default;

}  // namespace ast
}  // namespace joana

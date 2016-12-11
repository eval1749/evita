// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/statements.h"

#include "joana/public/ast/expressions.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace ast {

//
// BlockStatement
//
BlockStatement::BlockStatement(const Punctuator& semi_colon)
    : Statement(semi_colon.range()) {
  DCHECK_EQ(semi_colon.kind(), PunctuatorKind::LeftBrace);
}

BlockStatement::~BlockStatement() = default;

//
// EmptyStatement
//
EmptyStatement::EmptyStatement(const Punctuator& semi_colon)
    : Statement(semi_colon.range()) {
  DCHECK_EQ(semi_colon.kind(), PunctuatorKind::SemiColon);
}

EmptyStatement::~EmptyStatement() = default;

//
// ExpressionStatement
//
ExpressionStatement::ExpressionStatement(const Expression& expression)
    : Statement(expression.range()) {
  NodeEditor().AppendChild(this, const_cast<Expression*>(&expression));
}

ExpressionStatement::~ExpressionStatement() = default;

const Expression& ExpressionStatement::expression() const {
  auto* const child = NodeTraversal::FirstChildOf(*this);
  DCHECK(child) << *this;
  return child->As<Expression>();
}

//
// IfStatement
//
IfStatement::IfStatement(const Name& if_keyword,
                         const Expression& condition,
                         const Statement& then_clause,
                         const Statement& else_clause)
    : IfStatement(if_keyword, condition, then_clause) {
  NodeEditor().AppendChild(this, const_cast<Statement*>(&else_clause));
}

IfStatement::IfStatement(const Name& if_keyword,
                         const Expression& condition,
                         const Statement& then_clause)
    : Statement(if_keyword.range()) {
  DCHECK_EQ(if_keyword, NameId::If);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&then_clause));
}

IfStatement::~IfStatement() = default;

ast::Expression& IfStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

ast::Statement& IfStatement::else_clause() const {
  return NodeTraversal::ChildAt(*this, 2).As<Statement>();
}

ast::Statement& IfStatement::then_clause() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

bool IfStatement::has_else() const {
  return NodeTraversal::CountChildren(*this) == 3;
}

//
// InvalidStatement
//
InvalidStatement::InvalidStatement(const Node& node, int error_code)
    : Statement(node.range()), error_code_(error_code) {}

InvalidStatement::~InvalidStatement() = default;

//
// Statement
//
Statement::Statement(const SourceCodeRange& range) : ContainerNode(range) {}

Statement::~Statement() = default;

}  // namespace ast
}  // namespace joana

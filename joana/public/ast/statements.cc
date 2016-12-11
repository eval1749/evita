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
// BreakStatement
//
BreakStatement::BreakStatement(const Name& break_keyword, const Name& label)
    : BreakStatement(break_keyword) {
  DCHECK(!label.As<Name>().IsKeyword());
  NodeEditor().AppendChild(this, const_cast<Name*>(&label));
}

BreakStatement::BreakStatement(const Name& break_keyword)
    : Statement(break_keyword.range()) {
  DCHECK_EQ(break_keyword, NameId::Break);
}

BreakStatement::~BreakStatement() = default;

bool BreakStatement::has_label() const {
  return NodeTraversal::FirstChildOf(*this) != nullptr;
}

ast::Name& BreakStatement::label() const {
  DCHECK(has_label()) << *this;
  return NodeTraversal::FirstChildOf(*this)->As<Name>();
}

//
// ContinueStatement
//
ContinueStatement::ContinueStatement(const Name& continue_keyword,
                                     const Name& label)
    : ContinueStatement(continue_keyword) {
  DCHECK(!label.As<Name>().IsKeyword());
  NodeEditor().AppendChild(this, const_cast<Name*>(&label));
}

ContinueStatement::ContinueStatement(const Name& continue_keyword)
    : Statement(continue_keyword.range()) {
  DCHECK_EQ(continue_keyword, NameId::Continue);
}

ContinueStatement::~ContinueStatement() = default;

bool ContinueStatement::has_label() const {
  return NodeTraversal::FirstChildOf(*this) != nullptr;
}

ast::Name& ContinueStatement::label() const {
  DCHECK(has_label()) << *this;
  return NodeTraversal::FirstChildOf(*this)->As<Name>();
}

//
// DoStatement
//
DoStatement::DoStatement(const Name& keywod,
                         const Statement& statement,
                         const Expression& condition)
    : Statement(keywod.range()) {
  DCHECK_EQ(keywod, NameId::Do);
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
}

DoStatement::~DoStatement() = default;

ast::Expression& DoStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 1).As<Expression>();
}

ast::Statement& DoStatement::statement() const {
  return NodeTraversal::ChildAt(*this, 0).As<Statement>();
}

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
IfStatement::IfStatement(const Name& keywod,
                         const Expression& condition,
                         const Statement& then_clause,
                         const Statement& else_clause)
    : IfStatement(keywod, condition, then_clause) {
  NodeEditor().AppendChild(this, const_cast<Statement*>(&else_clause));
}

IfStatement::IfStatement(const Name& keywod,
                         const Expression& condition,
                         const Statement& then_clause)
    : Statement(keywod.range()) {
  DCHECK_EQ(keywod, NameId::If);
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

//
// ThrowStatement
//
ThrowStatement::ThrowStatement(const Name& keyword,
                               const Expression& expression)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Throw);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&expression));
}

ThrowStatement::~ThrowStatement() = default;

ast::Expression& ThrowStatement::expression() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

//
// WhileStatement
//
WhileStatement::WhileStatement(const Name& keywod,
                               const Expression& condition,
                               const Statement& statement)
    : Statement(keywod.range()) {
  DCHECK_EQ(keywod, NameId::While);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
}

WhileStatement::~WhileStatement() = default;

ast::Expression& WhileStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

ast::Statement& WhileStatement::statement() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

}  // namespace ast
}  // namespace joana

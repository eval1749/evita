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
BreakStatement::BreakStatement(const Name& keyword, const Name& label)
    : BreakStatement(keyword) {
  DCHECK(!label.As<Name>().IsKeyword());
  NodeEditor().AppendChild(this, const_cast<Name*>(&label));
}

BreakStatement::BreakStatement(const Name& keyword)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Break);
}

BreakStatement::~BreakStatement() = default;

bool BreakStatement::has_label() const {
  return NodeTraversal::FirstChildOf(*this) != nullptr;
}

Name& BreakStatement::label() const {
  DCHECK(has_label()) << *this;
  return NodeTraversal::FirstChildOf(*this)->As<Name>();
}

//
// CaseClause
//
CaseClause::CaseClause(const Name& keyword,
                       const Expression& expression,
                       const Statement& statement)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Case);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&expression));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
}

CaseClause::~CaseClause() = default;

Expression& CaseClause::expression() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

Statement& CaseClause::statement() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

//
// ContinueStatement
//
ContinueStatement::ContinueStatement(const Name& keyword, const Name& label)
    : ContinueStatement(keyword) {
  DCHECK(!label.As<Name>().IsKeyword());
  NodeEditor().AppendChild(this, const_cast<Name*>(&label));
}

ContinueStatement::ContinueStatement(const Name& keyword)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Continue);
}

ContinueStatement::~ContinueStatement() = default;

bool ContinueStatement::has_label() const {
  return NodeTraversal::FirstChildOf(*this) != nullptr;
}

Name& ContinueStatement::label() const {
  DCHECK(has_label()) << *this;
  return NodeTraversal::FirstChildOf(*this)->As<Name>();
}

//
// DoStatement
//
DoStatement::DoStatement(const Name& keyword,
                         const Statement& statement,
                         const Expression& condition)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Do);
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
}

DoStatement::~DoStatement() = default;

Expression& DoStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 1).As<Expression>();
}

Statement& DoStatement::statement() const {
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
IfStatement::IfStatement(const Name& keyword,
                         const Expression& condition,
                         const Statement& then_clause,
                         const Statement& else_clause)
    : IfStatement(keyword, condition, then_clause) {
  NodeEditor().AppendChild(this, const_cast<Statement*>(&else_clause));
}

IfStatement::IfStatement(const Name& keyword,
                         const Expression& condition,
                         const Statement& then_clause)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::If);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&then_clause));
}

IfStatement::~IfStatement() = default;

Expression& IfStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

Statement& IfStatement::else_clause() const {
  return NodeTraversal::ChildAt(*this, 2).As<Statement>();
}

Statement& IfStatement::then_clause() const {
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
// LabeledStatement
//
LabeledStatement::LabeledStatement(const Name& label,
                                   const Statement& statement)
    : Statement(label.range()) {
  NodeEditor().AppendChild(this, const_cast<Name*>(&label));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
}

LabeledStatement::~LabeledStatement() = default;

Name& LabeledStatement::label() const {
  return NodeTraversal::ChildAt(*this, 0).As<Name>();
}

Statement& LabeledStatement::statement() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

//
// Statement
//
Statement::Statement(const SourceCodeRange& range) : ContainerNode(range) {}

Statement::~Statement() = default;

//
// SwitchStatement
//
SwitchStatement::SwitchStatement(Zone* zone,
                                 const Name& keyword,
                                 const Expression& expression,
                                 const std::vector<Statement*>& clauses)
    : Statement(keyword.range()), clauses_(zone, clauses) {
  DCHECK_EQ(keyword, NameId::Switch);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&expression));
}

SwitchStatement::~SwitchStatement() = default;

Expression& SwitchStatement::expression() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

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

Expression& ThrowStatement::expression() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

//
// TryCatchStatement
//
TryCatchStatement::TryCatchStatement(const Name& keyword,
                                     const Statement& block,
                                     const Name& catch_name,
                                     const Statement& catch_block,
                                     const Statement& finally_block)
    : TryCatchStatement(keyword, block, catch_name, catch_block) {
  DCHECK_EQ(keyword, NameId::Try);
  NodeEditor().AppendChild(this, const_cast<Statement*>(&finally_block));
}

TryCatchStatement::TryCatchStatement(const Name& keyword,
                                     const Statement& block,
                                     const Name& catch_name,
                                     const Statement& catch_block)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Try);
  NodeEditor().AppendChild(this, const_cast<Statement*>(&block));
  NodeEditor().AppendChild(this, const_cast<Name*>(&catch_name));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&catch_block));
}

TryCatchStatement::~TryCatchStatement() = default;

Statement& TryCatchStatement::block() const {
  return NodeTraversal::ChildAt(*this, 0).As<Statement>();
}

Statement& TryCatchStatement::catch_block() const {
  return NodeTraversal::ChildAt(*this, 2).As<Statement>();
}

Name& TryCatchStatement::catch_name() const {
  return NodeTraversal::ChildAt(*this, 1).As<Name>();
}

Statement& TryCatchStatement::finally_block() const {
  return NodeTraversal::ChildAt(*this, 3).As<Statement>();
}

bool TryCatchStatement::has_finally() const {
  return NodeTraversal::CountChildren(*this) == 4;
}

//
// TryFinallyStatement
//
TryFinallyStatement::TryFinallyStatement(const Name& keyword,
                                         const Statement& block,
                                         const Statement& finally_block)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::Try);
  NodeEditor().AppendChild(this, const_cast<Statement*>(&block));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&finally_block));
}

TryFinallyStatement::~TryFinallyStatement() = default;

Statement& TryFinallyStatement::block() const {
  return NodeTraversal::ChildAt(*this, 0).As<Statement>();
}

Statement& TryFinallyStatement::finally_block() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

//
// WhileStatement
//
WhileStatement::WhileStatement(const Name& keyword,
                               const Expression& condition,
                               const Statement& statement)
    : Statement(keyword.range()) {
  DCHECK_EQ(keyword, NameId::While);
  NodeEditor().AppendChild(this, const_cast<Expression*>(&condition));
  NodeEditor().AppendChild(this, const_cast<Statement*>(&statement));
}

WhileStatement::~WhileStatement() = default;

Expression& WhileStatement::condition() const {
  return NodeTraversal::ChildAt(*this, 0).As<Expression>();
}

Statement& WhileStatement::statement() const {
  return NodeTraversal::ChildAt(*this, 1).As<Statement>();
}

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/statements.h"

#include "joana/public/ast/declarations.h"
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
BreakStatement::BreakStatement(const SourceCodeRange& range, Token* label)
    : NodeTemplate(label, range) {}

BreakStatement::~BreakStatement() = default;

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
// ConstStatement
//
ConstStatement::ConstStatement(const SourceCodeRange& range,
                               Expression* expression)
    : NodeTemplate(expression, range) {}

ConstStatement::~ConstStatement() = default;

//
// ContinueStatement
//
ContinueStatement::ContinueStatement(const SourceCodeRange& range, Token* label)
    : NodeTemplate(label, range) {}

ContinueStatement::~ContinueStatement() = default;

//
// DeclarationStatement
//
DeclarationStatement::DeclarationStatement(Declaration* declaration)
    : NodeTemplate(declaration, declaration->range()) {}

DeclarationStatement::~DeclarationStatement() = default;

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
// ForStatement
//
ForStatement::ForStatement(const SourceCodeRange& range,
                           Statement* init,
                           Expression* condition,
                           Expression* step,
                           Statement* body)
    : NodeTemplate(std::make_tuple(init, condition, step, body), range) {}

ForStatement::~ForStatement() = default;

//
// ForInStatement
//
ForInStatement::ForInStatement(const SourceCodeRange& range,
                               Statement* statement,
                               Statement* body)
    : NodeTemplate(std::make_tuple(statement, body), range) {}

ForInStatement::~ForInStatement() = default;

//
// ForOfStatement
//
ForOfStatement::ForOfStatement(const SourceCodeRange& range,
                               Statement* binding,
                               Expression* expression,
                               Statement* body)
    : NodeTemplate(std::make_tuple(binding, expression, body), range) {}

ForOfStatement::~ForOfStatement() = default;

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
// LetStatement
//
LetStatement::LetStatement(const SourceCodeRange& range, Expression* expression)
    : NodeTemplate(expression, range) {}

LetStatement::~LetStatement() = default;

//
// ReturnStatement
//
ReturnStatement::ReturnStatement(const SourceCodeRange& range,
                                 Expression* expression)
    : NodeTemplate(expression, range) {}

ReturnStatement::~ReturnStatement() = default;

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
// TryCatchFinallyStatement
//
TryCatchFinallyStatement::TryCatchFinallyStatement(const SourceCodeRange& range,
                                                   Statement* try_block,
                                                   Expression* catch_parameter,
                                                   Statement* catch_block,
                                                   Statement* finally_block)
    : NodeTemplate(std::make_tuple(try_block,
                                   catch_parameter,
                                   catch_block,
                                   finally_block),
                   range) {}

TryCatchFinallyStatement::~TryCatchFinallyStatement() = default;

//
// TryCatchStatement
//
TryCatchStatement::TryCatchStatement(const SourceCodeRange& range,
                                     Statement* try_block,
                                     Expression* catch_parameter,
                                     Statement* catch_block)
    : NodeTemplate(std::make_tuple(try_block, catch_parameter, catch_block),
                   range) {}

TryCatchStatement::~TryCatchStatement() = default;

//
// TryFinallyStatement
//
TryFinallyStatement::TryFinallyStatement(const SourceCodeRange& range,
                                         Statement* try_block,
                                         Statement* finally_block)
    : NodeTemplate(std::make_tuple(try_block, finally_block), range) {}

TryFinallyStatement::~TryFinallyStatement() = default;

//
// VarStatement
//
VarStatement::VarStatement(const SourceCodeRange& range, Expression* expression)
    : NodeTemplate(expression, range) {}

VarStatement::~VarStatement() = default;

//
// WhileStatement
//
WhileStatement::WhileStatement(const SourceCodeRange& range,
                               Expression* expression,
                               Statement* statement)
    : NodeTemplate(std::make_tuple(expression, statement), range) {}

WhileStatement::~WhileStatement() = default;

//
// WithStatement
//
WithStatement::WithStatement(const SourceCodeRange& range,
                             Expression* expression,
                             Statement* statement)
    : NodeTemplate(std::make_tuple(expression, statement), range) {}

WithStatement::~WithStatement() = default;

}  // namespace ast
}  // namespace joana

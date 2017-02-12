// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/statements.h"

#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Statement, BreakStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, CaseClause, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, ContinueStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, DoStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, EmptyStatement, 0)
IMPLEMENT_AST_SYNTAX_0(Statement, ForStatement, 5)
IMPLEMENT_AST_SYNTAX_0(Statement, ForInStatement, 4)
IMPLEMENT_AST_SYNTAX_0(Statement, ForOfStatement, 4)
IMPLEMENT_AST_SYNTAX_0(Statement, IfElseStatement, 3)
IMPLEMENT_AST_SYNTAX_0(Statement, IfStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, InvalidStatement, 0)
IMPLEMENT_AST_SYNTAX_0(Statement, LabeledStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, ReturnStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, ThrowStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, TryCatchStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, TryCatchFinallyStatement, 3)
IMPLEMENT_AST_SYNTAX_0(Statement, TryFinallyStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, WhileStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, WithStatement, 2)

//
// BlockStatement
//
BlockStatement::BlockStatement()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::BlockStatement,
                     Format::Builder().set_is_variadic(true).Build()) {}

BlockStatement::~BlockStatement() = default;

//
// CatchClause
//
CatchClause::CatchClause()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::CatchClause,
                     Format::Builder().set_arity(2).Build()) {}

CatchClause::~CatchClause() = default;

const Node& CatchClause::ParameterOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CatchClause);
  return node.child_at(0);
}

const Node& CatchClause::StatementOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::CatchClause);
  return node.child_at(1);
}

//
// ConstStatement
//
ConstStatement::ConstStatement()
    : VariableDeclaration(SyntaxCode::ConstStatement) {}

ConstStatement::~ConstStatement() = default;

//
// ExpressionStatement
//
ExpressionStatement::ExpressionStatement()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::ExpressionStatement,
                     Format::Builder().set_arity(1).Build()) {}

ExpressionStatement::~ExpressionStatement() = default;

const Node& ExpressionStatement::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ExpressionStatement);
  return node.child_at(0);
}

//
// LetStatement
//
LetStatement::LetStatement() : VariableDeclaration(SyntaxCode::LetStatement) {}

LetStatement::~LetStatement() = default;

//
// SwitchStatement
//
SwitchStatement::SwitchStatement()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::SwitchStatement,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

SwitchStatement::~SwitchStatement() = default;

ChildNodes SwitchStatement::ClausesOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::SwitchStatement);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& SwitchStatement::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::SwitchStatement);
  return node.child_at(0);
}

//
// VariableDeclaration
//
VariableDeclaration::VariableDeclaration(SyntaxCode syntax_code)
    : SyntaxTemplate(std::tuple<>(),
                     syntax_code,
                     Format::Builder().set_is_variadic(true).Build()) {}

VariableDeclaration::~VariableDeclaration() = default;

//
// VarStatement
//
VarStatement::VarStatement() : VariableDeclaration(SyntaxCode::VarStatement) {}

VarStatement::~VarStatement() = default;

}  // namespace ast
}  // namespace joana

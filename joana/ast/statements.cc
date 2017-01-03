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
IMPLEMENT_AST_SYNTAX_0(Statement, ExpressionStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, ForStatement, 4)
IMPLEMENT_AST_SYNTAX_0(Statement, ForInStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, ForOfStatement, 3)
IMPLEMENT_AST_SYNTAX_0(Statement, IfElseStatement, 3)
IMPLEMENT_AST_SYNTAX_0(Statement, IfStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, InvalidStatement, 0)
IMPLEMENT_AST_SYNTAX_0(Statement, LabeledStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, ReturnStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, ThrowStatement, 1)
IMPLEMENT_AST_SYNTAX_0(Statement, TryCatchStatement, 3)
IMPLEMENT_AST_SYNTAX_0(Statement, TryCatchFinallyStatement, 4)
IMPLEMENT_AST_SYNTAX_0(Statement, TryFinallyStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, WhileStatement, 2)
IMPLEMENT_AST_SYNTAX_0(Statement, WithStatement, 2)

//
// BlockStatementSyntax
//
BlockStatementSyntax::BlockStatementSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::BlockStatement,
                     Format::Builder().set_is_variadic(true).Build()) {}

BlockStatementSyntax::~BlockStatementSyntax() = default;

//
// ConstStatementSyntax
//
ConstStatementSyntax::ConstStatementSyntax()
    : VariableDeclaration(SyntaxCode::ConstStatement) {}

ConstStatementSyntax::~ConstStatementSyntax() = default;

//
// LetStatementSyntax
//
LetStatementSyntax::LetStatementSyntax()
    : VariableDeclaration(SyntaxCode::LetStatement) {}

LetStatementSyntax::~LetStatementSyntax() = default;

//
// SwitchStatementSyntax
//
SwitchStatementSyntax::SwitchStatementSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::SwitchStatement,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

SwitchStatementSyntax::~SwitchStatementSyntax() = default;

ChildNodes SwitchStatementSyntax::ClausesOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::SwitchStatement);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& SwitchStatementSyntax::ExpressionOf(const Node& node) {
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
// VarStatementSyntax
//
VarStatementSyntax::VarStatementSyntax()
    : VariableDeclaration(SyntaxCode::VarStatement) {}

VarStatementSyntax::~VarStatementSyntax() = default;

}  // namespace ast
}  // namespace joana

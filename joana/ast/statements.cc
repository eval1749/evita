// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/statements.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/node_editor.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace ast {

//
// BlockStatement
//
BlockStatement::BlockStatement(const SourceCodeRange& range,
                               StatementList* statements)
    : NodeTemplate(statements, range) {}

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
CaseClause::CaseClause(const SourceCodeRange& range,
                       Expression* expression,
                       Statement* statement)
    : NodeTemplate(std::make_tuple(expression, statement), range) {}

CaseClause::~CaseClause() = default;

//
// ConstStatement
//
ConstStatement::ConstStatement(
    const SourceCodeRange& range,
    const std::vector<const BindingElement*>& elements)
    : VariableDeclaration(range, elements) {}

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
DoStatement::DoStatement(const SourceCodeRange& range,
                         Statement* statement,
                         Expression* expression)
    : NodeTemplate(std::make_tuple(statement, expression), range) {}

DoStatement::~DoStatement() = default;

//
// EmptyStatement
//
EmptyStatement::EmptyStatement(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

EmptyStatement::~EmptyStatement() = default;

//
// ExpressionStatement
//
ExpressionStatement::ExpressionStatement(Expression* expression)
    : NodeTemplate(expression, expression->range()) {}

ExpressionStatement::~ExpressionStatement() = default;

//
// ForStatement
//
ForStatement::ForStatement(const SourceCodeRange& range,
                           Token* keyword,
                           Expression* init,
                           Expression* condition,
                           Expression* step,
                           Statement* body)
    : NodeTemplate(std::make_tuple(keyword, init, condition, step, body),
                   range) {}

ForStatement::~ForStatement() = default;

//
// ForInStatement
//
ForInStatement::ForInStatement(const SourceCodeRange& range,
                               Token* keyword,
                               Expression* expression,
                               Statement* body)
    : NodeTemplate(std::make_tuple(keyword, expression, body), range) {}

ForInStatement::~ForInStatement() = default;

//
// ForOfStatement
//
ForOfStatement::ForOfStatement(const SourceCodeRange& range,
                               Token* keyword,
                               Expression* binding,
                               Expression* expression,
                               Statement* body)
    : NodeTemplate(std::make_tuple(keyword, binding, expression, body), range) {
}

ForOfStatement::~ForOfStatement() = default;

//
// IfElseStatement
//
IfElseStatement::IfElseStatement(const SourceCodeRange& range,
                                 Expression* expression,
                                 Statement* then_clause,
                                 Statement* else_clause)
    : NodeTemplate(std::make_tuple(expression, then_clause, else_clause),
                   range) {}

IfElseStatement::~IfElseStatement() = default;

//
// IfStatement
//
IfStatement::IfStatement(const SourceCodeRange& range,
                         Expression* expression,
                         Statement* then_clause)
    : NodeTemplate(std::make_tuple(expression, then_clause), range) {}

IfStatement::~IfStatement() = default;

//
// InvalidStatement
//
InvalidStatement::InvalidStatement(const SourceCodeRange& range, int error_code)
    : NodeTemplate(error_code, range) {}

InvalidStatement::~InvalidStatement() = default;

//
// LabeledStatement
//
LabeledStatement::LabeledStatement(const SourceCodeRange& range,
                                   Name* label,
                                   Statement* statement)
    : NodeTemplate(std::make_tuple(label, statement), range) {}

LabeledStatement::~LabeledStatement() = default;

//
// LetStatement
//
LetStatement::LetStatement(const SourceCodeRange& range,
                           const std::vector<const BindingElement*>& elements)
    : VariableDeclaration(range, elements) {}

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
Statement::Statement(const SourceCodeRange& range) : Node(range) {}

Statement::~Statement() = default;

//
// StatementList
//
StatementList::StatementList(Zone* zone,
                             const std::vector<const Statement*>& statements)
    : statements_(zone, statements) {}

StatementList::~StatementList() = default;

//
// SwitchStatement
//
SwitchStatement::SwitchStatement(const SourceCodeRange& range,
                                 Expression* expression,
                                 StatementList* clauses)
    : NodeTemplate(std::make_tuple(expression, clauses), range) {}

SwitchStatement::~SwitchStatement() = default;

//
// ThrowStatement
//
ThrowStatement::ThrowStatement(const SourceCodeRange& range,
                               Expression* expression)
    : NodeTemplate(expression, range) {}

ThrowStatement::~ThrowStatement() = default;

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
// VariableDeclaration
//
VariableDeclaration::VariableDeclaration(
    const SourceCodeRange& range,
    const std::vector<const BindingElement*>& elements)
    : Statement(range), elements_(elements) {}

VariableDeclaration::~VariableDeclaration() = default;

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

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_STATEMENTS_H_
#define JOANA_AST_STATEMENTS_H_

#include "joana/ast/syntax.h"

namespace joana {
namespace ast {

class ChildNodes;
class Node;

DECLARE_AST_SYNTAX_0(BlockStatement)
DECLARE_AST_SYNTAX_0(BreakStatement)
DECLARE_AST_SYNTAX_0(CaseClause)
DECLARE_AST_SYNTAX_0(ContinueStatement)
DECLARE_AST_SYNTAX_0(DoStatement)
DECLARE_AST_SYNTAX_0(EmptyStatement)
DECLARE_AST_SYNTAX_0(ForStatement)
DECLARE_AST_SYNTAX_0(ForInStatement)
DECLARE_AST_SYNTAX_0(ForOfStatement)
DECLARE_AST_SYNTAX_0(IfElseStatement)
DECLARE_AST_SYNTAX_0(IfStatement)
DECLARE_AST_SYNTAX_0(InvalidStatement)
DECLARE_AST_SYNTAX_0(LabeledStatement)
DECLARE_AST_SYNTAX_0(ReturnStatement)
DECLARE_AST_SYNTAX_0(ThrowStatement)
DECLARE_AST_SYNTAX_0(TryCatchStatement)
DECLARE_AST_SYNTAX_0(TryCatchFinallyStatement)
DECLARE_AST_SYNTAX_0(TryFinallyStatement)
DECLARE_AST_SYNTAX_0(WhileStatement)
DECLARE_AST_SYNTAX_0(WithStatement)

//
// VariableDeclaration
//
class JOANA_AST_EXPORT VariableDeclaration : public SyntaxTemplate<Syntax> {
  DECLARE_ABSTRACT_AST_SYNTAX(VariableDeclaration, Syntax);

 public:
  ~VariableDeclaration() override;

 protected:
  explicit VariableDeclaration(SyntaxCode syntax_code);

 private:
  DISALLOW_COPY_AND_ASSIGN(VariableDeclaration);
};

//
// CatchClause
//
class JOANA_AST_EXPORT CatchClause final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(CatchClause, Syntax);

 public:
  ~CatchClause() final;

  static const ast::Node& ParameterOf(const ast::Node& node);
  static const ast::Node& StatementOf(const ast::Node& node);

 private:
  CatchClause();

  DISALLOW_COPY_AND_ASSIGN(CatchClause);
};

//
// ConstStatement
//
class JOANA_AST_EXPORT ConstStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(ConstStatement, VariableDeclaration);

 public:
  ~ConstStatement() final;

 private:
  ConstStatement();

  DISALLOW_COPY_AND_ASSIGN(ConstStatement);
};

//
// ExpressionStatement
//
class JOANA_AST_EXPORT ExpressionStatement final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ExpressionStatement, Syntax);

 public:
  ~ExpressionStatement() final;

  static const ast::Node& ExpressionOf(const ast::Node& node);

 private:
  ExpressionStatement();

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

//
// LetStatement
//
class JOANA_AST_EXPORT LetStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(LetStatement, VariableDeclaration);

 public:
  ~LetStatement() final;

 private:
  LetStatement();

  DISALLOW_COPY_AND_ASSIGN(LetStatement);
};

//
// SwitchStatement
//
class JOANA_AST_EXPORT SwitchStatement final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(SwitchStatement, Syntax);

 public:
  ~SwitchStatement() final;

  static ChildNodes ClausesOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  SwitchStatement();

  DISALLOW_COPY_AND_ASSIGN(SwitchStatement);
};

//
// VarStatement
//
class JOANA_AST_EXPORT VarStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(VarStatement, VariableDeclaration);

 public:
  ~VarStatement() final;

 private:
  VarStatement();

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_STATEMENTS_H_

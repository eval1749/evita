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
DECLARE_AST_SYNTAX_0(ExpressionStatement)
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
// ConstStatementSyntax
//
class JOANA_AST_EXPORT ConstStatementSyntax final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(ConstStatement, VariableDeclaration);

 public:
  ~ConstStatementSyntax() final;

 private:
  ConstStatementSyntax();

  DISALLOW_COPY_AND_ASSIGN(ConstStatementSyntax);
};

//
// LetStatementSyntax
//
class JOANA_AST_EXPORT LetStatementSyntax final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(LetStatement, VariableDeclaration);

 public:
  ~LetStatementSyntax() final;

 private:
  LetStatementSyntax();

  DISALLOW_COPY_AND_ASSIGN(LetStatementSyntax);
};

//
// SwitchStatementSyntax
//
class JOANA_AST_EXPORT SwitchStatementSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(SwitchStatement, Syntax);

 public:
  ~SwitchStatementSyntax() final;

  static ChildNodes ClausesOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  SwitchStatementSyntax();

  DISALLOW_COPY_AND_ASSIGN(SwitchStatementSyntax);
};

//
// VarStatementSyntax
//
class JOANA_AST_EXPORT VarStatementSyntax final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_SYNTAX(VarStatement, VariableDeclaration);

 public:
  ~VarStatementSyntax() final;

 private:
  VarStatementSyntax();

  DISALLOW_COPY_AND_ASSIGN(VarStatementSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_STATEMENTS_H_

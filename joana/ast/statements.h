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
DECLARE_AST_SYNTAX_0(ConstStatement)
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
DECLARE_AST_SYNTAX_0(LetStatement)
DECLARE_AST_SYNTAX_0(ReturnStatement)
DECLARE_AST_SYNTAX_0(ThrowStatement)
DECLARE_AST_SYNTAX_0(TryCatchStatement)
DECLARE_AST_SYNTAX_0(TryCatchFinallyStatement)
DECLARE_AST_SYNTAX_0(TryFinallyStatement)
DECLARE_AST_SYNTAX_0(VarStatement)
DECLARE_AST_SYNTAX_0(WhileStatement)
DECLARE_AST_SYNTAX_0(WithStatement)

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

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_STATEMENTS_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_STATEMENTS_H_
#define JOANA_PUBLIC_AST_STATEMENTS_H_

#include <vector>

#include "joana/public/ast/container_node.h"
#include "joana/public/ast/node_forward.h"
#include "joana/public/memory/zone_vector.h"

namespace joana {
namespace ast {

class Declaration;
class Expression;

//
// Statement
//
class JOANA_PUBLIC_EXPORT Statement : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE(Statement, ContainerNode);

 public:
  ~Statement() override;

 protected:
  explicit Statement(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

//
// BlockStatement
//
class JOANA_PUBLIC_EXPORT BlockStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(BlockStatement, Statement);

 public:
  ~BlockStatement() override;

 protected:
  explicit BlockStatement(const Punctuator& left_brace);

 private:
  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

//
// BreakStatement
//
class JOANA_PUBLIC_EXPORT BreakStatement
    : public NodeTemplate<Statement, Token*> {
  DECLARE_CONCRETE_AST_NODE(BreakStatement, Statement);

 public:
  ~BreakStatement() override;

  Token& label() const { return *member_at<0>(); }

 protected:
  BreakStatement(const SourceCodeRange& range, Token* label);

 private:
  DISALLOW_COPY_AND_ASSIGN(BreakStatement);
};

//
// CaseClause
//
class JOANA_PUBLIC_EXPORT CaseClause : public Statement {
  DECLARE_CONCRETE_AST_NODE(CaseClause, Statement);

 public:
  ~CaseClause() override;

  Expression& expression() const;
  Statement& statement() const;

 private:
  CaseClause(const Name& keyword,
             const Expression& expression,
             const Statement& statement);

  DISALLOW_COPY_AND_ASSIGN(CaseClause);
};

//
// ConstStatement
//
class JOANA_PUBLIC_EXPORT ConstStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ConstStatement, Statement);

 public:
  ~ConstStatement() override;

  const Expression& expression() { return *member_at<0>(); }

 protected:
  ConstStatement(const SourceCodeRange& range, Expression* expression);

 private:
  DISALLOW_COPY_AND_ASSIGN(ConstStatement);
};

//
// ContinueStatement
//
class JOANA_PUBLIC_EXPORT ContinueStatement
    : public NodeTemplate<Statement, Token*> {
  DECLARE_CONCRETE_AST_NODE(ContinueStatement, Statement);

 public:
  ~ContinueStatement() override;

  Token& label() const { return *member_at<0>(); }

 protected:
  ContinueStatement(const SourceCodeRange& range, Token* label);

 private:
  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

//
// DeclarationStatement wraps function and class.
//
class JOANA_PUBLIC_EXPORT DeclarationStatement final
    : public NodeTemplate<Statement, Declaration*> {
  DECLARE_CONCRETE_AST_NODE(DeclarationStatement, Statement);

 public:
  ~DeclarationStatement() final;

  const Declaration& declaration() const { return *member_at<0>(); }

 private:
  explicit DeclarationStatement(Declaration* declaration);

  DISALLOW_COPY_AND_ASSIGN(DeclarationStatement);
};

//
// DoStatement
//
class JOANA_PUBLIC_EXPORT DoStatement
    : public NodeTemplate<Statement, Statement*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(DoStatement, Statement);

 public:
  ~DoStatement() final;

  Statement& statement() const { return *member_at<0>(); }
  Expression& expression() const { return *member_at<1>(); }

 protected:
  DoStatement(const SourceCodeRange& range,
              Statement* statement,
              Expression* expression);

 private:
  DISALLOW_COPY_AND_ASSIGN(DoStatement);
};

//
// EmptyStatement
//
class JOANA_PUBLIC_EXPORT EmptyStatement : public NodeTemplate<Statement> {
  DECLARE_CONCRETE_AST_NODE(EmptyStatement, Statement);

 public:
  ~EmptyStatement() override;

 protected:
  explicit EmptyStatement(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

//
// ExpressionStatement
//
class JOANA_PUBLIC_EXPORT ExpressionStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ExpressionStatement, Statement);

 public:
  ~ExpressionStatement() override;

  const Expression& expression() const { return *member_at<0>(); }

 private:
  explicit ExpressionStatement(Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

//
// ForStatement
//
class JOANA_PUBLIC_EXPORT ForStatement : public NodeTemplate<Statement,
                                                             Statement*,
                                                             Expression*,
                                                             Expression*,
                                                             Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForStatement, Statement);

 public:
  ~ForStatement() override;

  const Expression& condition() { return *member_at<1>(); }
  const Statement& body() { return *member_at<3>(); }
  const Statement& init() { return *member_at<0>(); }
  const Expression& step() { return *member_at<2>(); }

 protected:
  ForStatement(const SourceCodeRange& range,
               Statement* init,
               Expression* condition,
               Expression* step,
               Statement* body);

 private:
  DISALLOW_COPY_AND_ASSIGN(ForStatement);
};

//
// ForInStatement
//  |statement| is one of
//    'const' binding 'in' expression
//    'let' binding 'in' expression
//    'var' binding 'in' expression
//    left-hand-side-expression 'in' expression
//
class JOANA_PUBLIC_EXPORT ForInStatement
    : public NodeTemplate<Statement, Statement*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForInStatement, Statement);

 public:
  ~ForInStatement() override;

  const Statement& body() { return *member_at<1>(); }
  const Statement& statement() { return *member_at<0>(); }

 protected:
  ForInStatement(const SourceCodeRange& range,
                 Statement* statement,
                 Statement* body);

 private:
  DISALLOW_COPY_AND_ASSIGN(ForInStatement);
};

//
// ForOfStatement
//
class JOANA_PUBLIC_EXPORT ForOfStatement
    : public NodeTemplate<Statement, Statement*, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForOfStatement, Statement);

 public:
  ~ForOfStatement() override;

  const Statement& body() { return *member_at<2>(); }
  const Statement& binding() { return *member_at<0>(); }
  const Expression& expression() { return *member_at<1>(); }

 protected:
  ForOfStatement(const SourceCodeRange& range,
                 Statement* binding,
                 Expression* expression,
                 Statement* body);

 private:
  DISALLOW_COPY_AND_ASSIGN(ForOfStatement);
};

//
// IfElseStatement
//
class JOANA_PUBLIC_EXPORT IfElseStatement
    : public NodeTemplate<Statement, Expression*, Statement*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(IfElseStatement, Statement);

 public:
  ~IfElseStatement() override;

  Statement& else_clause() const { return *member_at<2>(); }
  Expression& expression() const { return *member_at<0>(); }
  Statement& then_clause() const { return *member_at<1>(); }

 private:
  IfElseStatement(const SourceCodeRange& range,
                  Expression* Expression,
                  Statement* then_clause,
                  Statement* else_clause);

  DISALLOW_COPY_AND_ASSIGN(IfElseStatement);
};

//
// IfStatement
//
class JOANA_PUBLIC_EXPORT IfStatement
    : public NodeTemplate<Statement, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(IfStatement, Statement);

 public:
  ~IfStatement() override;

  Expression& expression() const { return *member_at<0>(); }
  Statement& then_clause() const { return *member_at<1>(); }

 private:
  IfStatement(const SourceCodeRange& range,
              Expression* Expression,
              Statement* then_clause);

  DISALLOW_COPY_AND_ASSIGN(IfStatement);
};

//
// InvalidStatement
//
class JOANA_PUBLIC_EXPORT InvalidStatement
    : public NodeTemplate<Statement, int> {
  DECLARE_CONCRETE_AST_NODE(InvalidStatement, Statement);

 public:
  ~InvalidStatement() override;

  int error_code() const { return member_at<0>(); }

 private:
  InvalidStatement(const SourceCodeRange& range, int error_code);

  DISALLOW_COPY_AND_ASSIGN(InvalidStatement);
};

//
// LabeledStatement
//
class JOANA_PUBLIC_EXPORT LabeledStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(LabeledStatement, Statement);

 public:
  ~LabeledStatement() override;

  Name& label() const;
  Statement& statement() const;

 private:
  LabeledStatement(const Name& label, const Statement& statement);

  DISALLOW_COPY_AND_ASSIGN(LabeledStatement);
};

//
// LetStatement
//
class JOANA_PUBLIC_EXPORT LetStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(LetStatement, Statement);

 public:
  ~LetStatement() override;

  const Expression& expression() { return *member_at<0>(); }

 protected:
  LetStatement(const SourceCodeRange& range, Expression* expression);

 private:
  DISALLOW_COPY_AND_ASSIGN(LetStatement);
};

//
// ReturnStatement
//
class JOANA_PUBLIC_EXPORT ReturnStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ReturnStatement, Statement);

 public:
  ~ReturnStatement() override;

  // Returns an expression which may be |ElisionExpression|.
  const Expression& expression() const { return *member_at<0>(); }

 private:
  ReturnStatement(const SourceCodeRange& range, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

//
// SwitchStatement
//
class JOANA_PUBLIC_EXPORT SwitchStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(SwitchStatement, Statement);

 public:
  ~SwitchStatement() override;

  Expression& expression() const;
  const ZoneVector<Statement*>& clauses() const { return clauses_; }

 private:
  SwitchStatement(Zone* zone,
                  const Name& keyword,
                  const Expression& condition,
                  const std::vector<Statement*>& clauses);

  ZoneVector<Statement*> clauses_;

  DISALLOW_COPY_AND_ASSIGN(SwitchStatement);
};

//
// ThrowStatement
//
class JOANA_PUBLIC_EXPORT ThrowStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(ThrowStatement, Statement);

 public:
  ~ThrowStatement() override;

  Expression& expression() const;

 private:
  ThrowStatement(const Name& keyword, const Expression& expression);

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

//
// TryCatchFinallyStatement
//
class JOANA_PUBLIC_EXPORT TryCatchFinallyStatement
    : public NodeTemplate<Statement,
                          Statement*,
                          Expression*,
                          Statement*,
                          Statement*> {
  DECLARE_CONCRETE_AST_NODE(TryCatchFinallyStatement, Statement);

 public:
  ~TryCatchFinallyStatement() final;

  Statement& catch_block() const { return *member_at<2>(); }
  Expression& catch_parameter() const { return *member_at<1>(); }
  Statement& finally_block() const { return *member_at<3>(); }
  Statement& try_block() const { return *member_at<0>(); }

 protected:
  TryCatchFinallyStatement(const SourceCodeRange& range,
                           Statement* try_block,
                           Expression* catch_parameter,
                           Statement* catch_block,
                           Statement* finally_block);

 private:
  DISALLOW_COPY_AND_ASSIGN(TryCatchFinallyStatement);
};

//
// TryCatchStatement
//
class JOANA_PUBLIC_EXPORT TryCatchStatement
    : public NodeTemplate<Statement, Statement*, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(TryCatchStatement, Statement);

 public:
  ~TryCatchStatement() final;

  Statement& catch_block() const { return *member_at<2>(); }
  Expression& catch_parameter() const { return *member_at<1>(); }
  Statement& try_block() const { return *member_at<0>(); }

 protected:
  TryCatchStatement(const SourceCodeRange& range,
                    Statement* try_block,
                    Expression* catch_parameter,
                    Statement* catch_block);

 private:
  DISALLOW_COPY_AND_ASSIGN(TryCatchStatement);
};

//
// TryFinallyStatement
//
class JOANA_PUBLIC_EXPORT TryFinallyStatement
    : public NodeTemplate<Statement, Statement*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(TryFinallyStatement, Statement);

 public:
  ~TryFinallyStatement() final;

  Statement& finally_block() const { return *member_at<1>(); }
  Statement& try_block() const { return *member_at<0>(); }

 protected:
  TryFinallyStatement(const SourceCodeRange& range,
                      Statement* try_block,
                      Statement* finally_block);

 private:
  DISALLOW_COPY_AND_ASSIGN(TryFinallyStatement);
};

//
// VarStatement
//
class JOANA_PUBLIC_EXPORT VarStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(VarStatement, Statement);

 public:
  ~VarStatement() override;

  const Expression& expression() { return *member_at<0>(); }

 protected:
  VarStatement(const SourceCodeRange& range, Expression* expression);

 private:
  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

//
// WhileStatement
//
class JOANA_PUBLIC_EXPORT WhileStatement
    : public NodeTemplate<Statement, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(WhileStatement, Statement);

 public:
  ~WhileStatement() final;

  Statement& statement() const { return *member_at<1>(); }
  Expression& expression() const { return *member_at<0>(); }

 protected:
  WhileStatement(const SourceCodeRange& range,
                 Expression* expression,
                 Statement* statement);

 private:
  DISALLOW_COPY_AND_ASSIGN(WhileStatement);
};

//
// WithStatement
//
class JOANA_PUBLIC_EXPORT WithStatement
    : public NodeTemplate<Statement, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(WithStatement, Statement);

 public:
  ~WithStatement() final;

  Statement& statement() const { return *member_at<1>(); }
  Expression& expression() const { return *member_at<0>(); }

 protected:
  WithStatement(const SourceCodeRange& range,
                Expression* expression,
                Statement* statement);

 private:
  DISALLOW_COPY_AND_ASSIGN(WithStatement);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_STATEMENTS_H_

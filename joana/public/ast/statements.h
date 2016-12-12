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
class JOANA_PUBLIC_EXPORT BreakStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(BreakStatement, Statement);

 public:
  ~BreakStatement() override;

  bool has_label() const;
  Name& label() const;

 protected:
  BreakStatement(const Name& keyword, const Name& label);
  explicit BreakStatement(const Name& keyword);

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
// ContinueStatement
//
class JOANA_PUBLIC_EXPORT ContinueStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(ContinueStatement, Statement);

 public:
  ~ContinueStatement() override;

  bool has_label() const;
  Name& label() const;

 protected:
  ContinueStatement(const Name& keyword, const Name& label);
  explicit ContinueStatement(const Name& keyword);

 private:
  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

//
// DoStatement
//
class JOANA_PUBLIC_EXPORT DoStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(DoStatement, Statement);

 public:
  ~DoStatement() override;

  Expression& condition() const;
  Statement& statement() const;

 protected:
  explicit DoStatement(const Name& keyword,
                       const Statement& statement,
                       const Expression& condition);

 private:
  DISALLOW_COPY_AND_ASSIGN(DoStatement);
};

//
// EmptyStatement
//
class JOANA_PUBLIC_EXPORT EmptyStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(EmptyStatement, Statement);

 public:
  ~EmptyStatement() override;

 protected:
  explicit EmptyStatement(const Punctuator& semi_colon);

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

//
// ExpressionStatement
//
class JOANA_PUBLIC_EXPORT ExpressionStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(ExpressionStatement, Statement);

 public:
  ~ExpressionStatement() override;

  const Expression& expression() const;

 private:
  explicit ExpressionStatement(const Expression& expression);

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

//
// IfStatement
//
class JOANA_PUBLIC_EXPORT IfStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(IfStatement, Statement);

 public:
  ~IfStatement() override;

  ast::Expression& condition() const;
  ast::Statement& else_clause() const;
  bool has_else() const;
  ast::Statement& then_clause() const;

 private:
  IfStatement(const Name& keyword,
              const Expression& condition,
              const Statement& then_clause,
              const Statement& else_clause);
  IfStatement(const Name& keyword,
              const Expression& condition,
              const Statement& then_clause);

  DISALLOW_COPY_AND_ASSIGN(IfStatement);
};

//
// InvalidStatement
//
class JOANA_PUBLIC_EXPORT InvalidStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(InvalidStatement, Statement);

 public:
  ~InvalidStatement() override;

  int error_code() const { return error_code_; }

 private:
  InvalidStatement(const Node& node, int error_code);

  const int error_code_;

  DISALLOW_COPY_AND_ASSIGN(InvalidStatement);
};

//
// LabeledStatement
//
class JOANA_PUBLIC_EXPORT LabeledStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(LabeledStatement, Statement);

 public:
  ~LabeledStatement() override;

  ast::Name& label() const;
  ast::Statement& statement() const;

 private:
  LabeledStatement(const Name& label, const Statement& statement);

  DISALLOW_COPY_AND_ASSIGN(LabeledStatement);
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

  ast::Expression& expression() const;

 private:
  ThrowStatement(const Name& keyword, const Expression& expression);

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

//
// TryCatchStatement
//
class JOANA_PUBLIC_EXPORT TryCatchStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(TryCatchStatement, Statement);

 public:
  ~TryCatchStatement() override;

  Statement& block() const;
  Statement& catch_block() const;
  Name& catch_name() const;
  Statement& finally_block() const;
  bool has_finally() const;

 protected:
  TryCatchStatement(const Name& keyword,
                    const Statement& block,
                    const Name& catch_name,
                    const Statement& catch_block,
                    const Statement& finally_block);
  TryCatchStatement(const Name& keyword,
                    const Statement& block,
                    const Name& catch_name,
                    const Statement& catch_block);

 private:
  DISALLOW_COPY_AND_ASSIGN(TryCatchStatement);
};

//
// TryFinallyStatement
//
class JOANA_PUBLIC_EXPORT TryFinallyStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(TryFinallyStatement, Statement);

 public:
  ~TryFinallyStatement() override;

  Statement& block() const;
  Statement& finally_block() const;

 protected:
  TryFinallyStatement(const Name& keyword,
                      const Statement& block,
                      const Statement& finally_block);

 private:
  DISALLOW_COPY_AND_ASSIGN(TryFinallyStatement);
};

//
// WhileStatement
//
class JOANA_PUBLIC_EXPORT WhileStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(WhileStatement, Statement);

 public:
  ~WhileStatement() override;

  Expression& condition() const;
  Statement& statement() const;

 protected:
  explicit WhileStatement(const Name& keyword,
                          const Expression& condition,
                          const Statement& statement);

 private:
  DISALLOW_COPY_AND_ASSIGN(WhileStatement);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_STATEMENTS_H_

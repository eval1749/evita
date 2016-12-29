// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_STATEMENTS_H_
#define JOANA_AST_STATEMENTS_H_

#include <vector>

#include "joana/ast/container_node.h"
#include "joana/ast/node_forward.h"
#include "joana/base/iterator_utils.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {
namespace ast {

class Declaration;
class Expression;

//
// Statement
//
class JOANA_AST_EXPORT Statement : public Node {
  DECLARE_ABSTRACT_AST_NODE(Statement, Node);

 public:
  ~Statement() override;

 protected:
  explicit Statement(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

//
// StatementList
//
class JOANA_AST_EXPORT StatementList final : public ZoneAllocated {
 public:
  ~StatementList();

  auto begin() const { return ReferenceRangeOf(statements_).begin(); }
  bool empty() const { return statements_.empty(); }
  auto end() const { return ReferenceRangeOf(statements_).end(); }
  size_t size() const { return statements_.size(); }

 private:
  friend class NodeFactory;

  StatementList(Zone* zone, const std::vector<const Statement*>& statements);

  ZoneVector<const Statement*> statements_;

  DISALLOW_COPY_AND_ASSIGN(StatementList);
};

//
// VariableDeclaration is a base class of ConstStatement, LetStatement and
// VarStatement to hold list of BindingElement
//
class JOANA_AST_EXPORT VariableDeclaration : public Statement {
  DECLARE_ABSTRACT_AST_NODE(VariableDeclaration, Statement);

 public:
  ~VariableDeclaration() override;

  const BindingElementList& elements() const { return elements_; }

 protected:
  VariableDeclaration(const SourceCodeRange& range,
                      const std::vector<const BindingElement*>& elements);

 private:
  NodeListTemplate<const BindingElement> elements_;

  DISALLOW_COPY_AND_ASSIGN(VariableDeclaration);
};

//
// BlockStatement
//
class JOANA_AST_EXPORT BlockStatement
    : public NodeTemplate<Statement, StatementList*> {
  DECLARE_CONCRETE_AST_NODE(BlockStatement, Statement);

 public:
  ~BlockStatement() override;

  const StatementList& statements() const { return *member_at<0>(); }

 protected:
  BlockStatement(const SourceCodeRange& range, StatementList* statements);

 private:
  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

//
// BreakStatement
//
class JOANA_AST_EXPORT BreakStatement : public NodeTemplate<Statement, Token*> {
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
class JOANA_AST_EXPORT CaseClause
    : public NodeTemplate<Statement, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(CaseClause, Statement);

 public:
  ~CaseClause() override;

  Expression& expression() const { return *member_at<0>(); }
  Statement& statement() const { return *member_at<1>(); }

 private:
  CaseClause(const SourceCodeRange& range,
             Expression* expression,
             Statement* statement);

  DISALLOW_COPY_AND_ASSIGN(CaseClause);
};

//
// ConstStatement
//
class JOANA_AST_EXPORT ConstStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(ConstStatement, VariableDeclaration);

 public:
  ~ConstStatement() final;

 protected:
  ConstStatement(const SourceCodeRange& range,
                 const std::vector<const BindingElement*>& elements);

 private:
  DISALLOW_COPY_AND_ASSIGN(ConstStatement);
};

//
// ContinueStatement
//
class JOANA_AST_EXPORT ContinueStatement
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
class JOANA_AST_EXPORT DeclarationStatement final
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
class JOANA_AST_EXPORT DoStatement
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
class JOANA_AST_EXPORT EmptyStatement : public NodeTemplate<Statement> {
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
class JOANA_AST_EXPORT ExpressionStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ExpressionStatement, Statement);

 public:
  ~ExpressionStatement() override;

  const Expression& expression() const { return *member_at<0>(); }

 private:
  ExpressionStatement(const SourceCodeRange& range, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

//
// ForStatement
//
class JOANA_AST_EXPORT ForStatement : public NodeTemplate<Statement,
                                                          Token*,
                                                          Expression*,
                                                          Expression*,
                                                          Expression*,
                                                          Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForStatement, Statement);

 public:
  ~ForStatement() override;

  const Expression& condition() const { return *member_at<2>(); }
  const Statement& body() const { return *member_at<4>(); }
  const Expression& init() const { return *member_at<1>(); }
  const Token& keyword() const { return *member_at<0>(); }
  const Expression& step() const { return *member_at<3>(); }

 protected:
  ForStatement(const SourceCodeRange& range,
               Token* keyword,
               Expression* init,
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
class JOANA_AST_EXPORT ForInStatement
    : public NodeTemplate<Statement, Token*, Expression*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForInStatement, Statement);

 public:
  ~ForInStatement() override;

  const Statement& body() const { return *member_at<2>(); }
  const Expression& expression() const { return *member_at<1>(); }
  const Token& keyword() const { return *member_at<0>(); }

 protected:
  ForInStatement(const SourceCodeRange& range,
                 Token* keyword,
                 Expression* expression,
                 Statement* body);

 private:
  DISALLOW_COPY_AND_ASSIGN(ForInStatement);
};

//
// ForOfStatement
//
class JOANA_AST_EXPORT ForOfStatement : public NodeTemplate<Statement,
                                                            Token*,
                                                            Expression*,
                                                            Expression*,
                                                            Statement*> {
  DECLARE_CONCRETE_AST_NODE(ForOfStatement, Statement);

 public:
  ~ForOfStatement() override;

  const Expression& binding() const { return *member_at<1>(); }
  const Statement& body() const { return *member_at<3>(); }
  const Expression& expression() const { return *member_at<2>(); }
  const Token& keyword() const { return *member_at<0>(); }

 protected:
  ForOfStatement(const SourceCodeRange& range,
                 Token* keyword,
                 Expression* binding,
                 Expression* expression,
                 Statement* body);

 private:
  DISALLOW_COPY_AND_ASSIGN(ForOfStatement);
};

//
// IfElseStatement
//
class JOANA_AST_EXPORT IfElseStatement
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
class JOANA_AST_EXPORT IfStatement
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
class JOANA_AST_EXPORT InvalidStatement : public NodeTemplate<Statement, int> {
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
class JOANA_AST_EXPORT LabeledStatement
    : public NodeTemplate<Statement, Name*, Statement*> {
  DECLARE_CONCRETE_AST_NODE(LabeledStatement, Statement);

 public:
  ~LabeledStatement() override;

  Name& label() const { return *member_at<0>(); }
  Statement& statement() const { return *member_at<1>(); }

 private:
  LabeledStatement(const SourceCodeRange& range,
                   Name* label,
                   Statement* statement);

  DISALLOW_COPY_AND_ASSIGN(LabeledStatement);
};

//
// LetStatement
//
class JOANA_AST_EXPORT LetStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(LetStatement, VariableDeclaration);

 public:
  ~LetStatement() final;

 protected:
  LetStatement(const SourceCodeRange& range,
               const std::vector<const BindingElement*>& elements);

 private:
  DISALLOW_COPY_AND_ASSIGN(LetStatement);
};

//
// ReturnStatement
//
class JOANA_AST_EXPORT ReturnStatement
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
class JOANA_AST_EXPORT SwitchStatement
    : public NodeTemplate<Statement, Expression*, StatementList*> {
  DECLARE_CONCRETE_AST_NODE(SwitchStatement, Statement);

 public:
  ~SwitchStatement() override;

  Expression& expression() const { return *member_at<0>(); }
  const StatementList& clauses() const { return *member_at<1>(); }

 private:
  SwitchStatement(const SourceCodeRange& range,
                  Expression* expression,
                  StatementList* clauses);

  DISALLOW_COPY_AND_ASSIGN(SwitchStatement);
};

//
// ThrowStatement
//
class JOANA_AST_EXPORT ThrowStatement
    : public NodeTemplate<Statement, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ThrowStatement, Statement);

 public:
  ~ThrowStatement() override;

  Expression& expression() const { return *member_at<0>(); }

 private:
  ThrowStatement(const SourceCodeRange& range, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

//
// TryCatchFinallyStatement
//
class JOANA_AST_EXPORT TryCatchFinallyStatement
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
class JOANA_AST_EXPORT TryCatchStatement
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
class JOANA_AST_EXPORT TryFinallyStatement
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
class JOANA_AST_EXPORT VarStatement final : public VariableDeclaration {
  DECLARE_CONCRETE_AST_NODE_WITH_LIST(VarStatement, VariableDeclaration);

 public:
  ~VarStatement() final;

 protected:
  VarStatement(const SourceCodeRange& range,
               const std::vector<const BindingElement*>& elements);

 private:
  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

//
// WhileStatement
//
class JOANA_AST_EXPORT WhileStatement
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
class JOANA_AST_EXPORT WithStatement
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

#endif  // JOANA_AST_STATEMENTS_H_

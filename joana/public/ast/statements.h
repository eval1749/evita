// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_STATEMENTS_H_
#define JOANA_PUBLIC_AST_STATEMENTS_H_

#include "joana/public/ast/container_node.h"
#include "joana/public/ast/node_forward.h"

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

  Expression* expression() const;

 private:
  explicit ExpressionStatement(const Expression& expression);

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
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

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_STATEMENTS_H_
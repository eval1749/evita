// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_EXPRESSIONS_H_
#define JOANA_PUBLIC_AST_EXPRESSIONS_H_

#include "joana/public/ast/container_node.h"
#include "joana/public/ast/node_forward.h"

namespace joana {
namespace ast {

class Literal;

class JOANA_PUBLIC_EXPORT Expression : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE(Expression, ContainerNode);

 public:
  ~Expression() override;

 protected:
  explicit Expression(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

//
// InvalidExpression
//
class JOANA_PUBLIC_EXPORT InvalidExpression : public Expression {
  DECLARE_CONCRETE_AST_NODE(InvalidExpression, Expression);

 public:
  ~InvalidExpression() override;

  int error_code() const { return error_code_; }

 private:
  InvalidExpression(const Node& node, int error_code);

  const int error_code_;

  DISALLOW_COPY_AND_ASSIGN(InvalidExpression);
};

//
// LiteralExpression
//
class JOANA_PUBLIC_EXPORT LiteralExpression : public Expression {
  DECLARE_CONCRETE_AST_NODE(LiteralExpression, Expression);

 public:
  ~LiteralExpression() override;

  const Literal& literal() const;

 private:
  explicit LiteralExpression(const Literal& literal);

  DISALLOW_COPY_AND_ASSIGN(LiteralExpression);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_EXPRESSIONS_H_

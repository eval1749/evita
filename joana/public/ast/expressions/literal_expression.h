// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_EXPRESSIONS_LITERAL_EXPRESSION_H_
#define JOANA_PUBLIC_AST_EXPRESSIONS_LITERAL_EXPRESSION_H_

#include "joana/public/ast/expressions/expression.h"

namespace joana {
namespace ast {

class Literal;

class JOANA_PUBLIC_EXPORT LiteralExpression : public Expression {
  DECLARE_CONCRETE_AST_NODE(LiteralExpression, Expression);

 public:
  ~LiteralExpression() override;

  const Literal& literal() const { return literal_; }

 private:
  explicit LiteralExpression(const Literal& literal);

  const Literal& literal_;

  DISALLOW_COPY_AND_ASSIGN(LiteralExpression);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_EXPRESSIONS_LITERAL_EXPRESSION_H_

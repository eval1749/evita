// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_NUMERIC_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_NUMERIC_LITERAL_H_

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT NumericLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(NumericLiteral, Literal);

 public:
  ~NumericLiteral() final;

  double value() const { return value_; }

 private:
  NumericLiteral(const SourceCodeRange& range, double value);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const double value_;

  DISALLOW_COPY_AND_ASSIGN(NumericLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_NUMERIC_LITERAL_H_

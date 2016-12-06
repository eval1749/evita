// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_BOOLEAN_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_BOOLEAN_LITERAL_H_

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT BooleanLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(BooleanLiteral, Literal);

 public:
  ~BooleanLiteral() final;

  bool value() const { return value_; }

 private:
  BooleanLiteral(const SourceCodeRange& range, bool value);

  const bool value_;

  DISALLOW_COPY_AND_ASSIGN(BooleanLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_BOOLEAN_LITERAL_H_

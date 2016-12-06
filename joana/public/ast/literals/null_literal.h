// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_NULL_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_NULL_LITERAL_H_

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT NullLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(NullLiteral, Literal);

 public:
  ~NullLiteral() final;

 private:
  explicit NullLiteral(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_NULL_LITERAL_H_

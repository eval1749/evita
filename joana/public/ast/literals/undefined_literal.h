// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_UNDEFINED_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_UNDEFINED_LITERAL_H_

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT UndefinedLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(UndefinedLiteral, Literal);

 public:
  ~UndefinedLiteral() final;

 private:
  explicit UndefinedLiteral(const SourceCodeRange& location);

  DISALLOW_COPY_AND_ASSIGN(UndefinedLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_UNDEFINED_LITERAL_H_

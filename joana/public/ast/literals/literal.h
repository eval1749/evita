// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_LITERAL_H_

#include "joana/public/ast/node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Literal : public Node {
  DECLARE_ABSTRACT_AST_NODE(Literal, Node);

 public:
  ~Literal() override;

 protected:
  explicit Literal(const SourceCodeRange& location);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_LITERAL_H_

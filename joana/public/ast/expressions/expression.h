// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_EXPRESSIONS_EXPRESSION_H_
#define JOANA_PUBLIC_AST_EXPRESSIONS_EXPRESSION_H_

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Expression : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE(Expression, ContainerNode);

 public:
  ~Expression() override;

 protected:
  explicit Expression(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_EXPRESSIONS_EXPRESSION_H_

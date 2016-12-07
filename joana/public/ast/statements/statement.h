// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_STATEMENTS_STATEMENT_H_
#define JOANA_PUBLIC_AST_STATEMENTS_STATEMENT_H_

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Statement : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE(Statement, ContainerNode);

 public:
  ~Statement() override;

 protected:
  explicit Statement(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_STATEMENTS_STATEMENT_H_

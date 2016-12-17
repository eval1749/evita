// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_MODULE_H_
#define JOANA_PUBLIC_AST_MODULE_H_

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Module final : public ContainerNode {
  DECLARE_CONCRETE_AST_NODE(Module, ContainerNode);

 public:
  ~Module() final;

 private:
  explicit Module(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(Module);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_MODULE_H_

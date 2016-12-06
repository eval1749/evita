// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_TEMPLATE_H_
#define JOANA_PUBLIC_AST_TEMPLATE_H_

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Template final : public ContainerNode {
  DECLARE_CONCRETE_AST_NODE(Template, ContainerNode);

 public:
  explicit Template(const SourceCodeRange& range);
  ~Template() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(Template);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_TEMPLATE_H_

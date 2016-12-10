// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_CONTAINER_NODE_H_
#define JOANA_PUBLIC_AST_CONTAINER_NODE_H_

#include "joana/public/ast/node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT ContainerNode : public Node {
  DECLARE_ABSTRACT_AST_NODE(ContainerNode, Node);

 public:
  ~ContainerNode() override;

  // |Node| implementations
  Node* first_child() const final;
  Node* last_child() const final;

 protected:
  explicit ContainerNode(const SourceCodeRange& range);

 private:
  Node* first_child_ = nullptr;
  Node* last_child_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ContainerNode);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_CONTAINER_NODE_H_

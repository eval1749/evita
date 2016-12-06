// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

ContainerNode::ContainerNode(const SourceCodeRange& range) : Node(range) {}

ContainerNode::~ContainerNode() = default;

Node* ContainerNode::first_child() const {
  return first_child_;
}
Node* ContainerNode::last_child() const {
  return last_child_;
}

}  // namespace ast
}  // namespace joana

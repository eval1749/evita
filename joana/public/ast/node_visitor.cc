// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/node_visitor.h"

#include "joana/public/ast/declarations.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace ast {

NodeVisitor::NodeVisitor() = default;
NodeVisitor::~NodeVisitor() = default;

void NodeVisitor::Visit(const Node& node) {
  Visit(const_cast<Node*>(&node));
}

void NodeVisitor::Visit(Node* node) {
  node->Accept(this);
}

#define V(name) \
  void name::Accept(NodeVisitor* visitor) { visitor->Visit##name(this); }
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

}  // namespace ast
}  // namespace joana

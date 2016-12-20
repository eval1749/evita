// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_visitor.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/literals.h"
#include "joana/ast/module.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"

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

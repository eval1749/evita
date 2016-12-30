// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_visitor.h"

#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_nodes.h"
#include "joana/ast/literals.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace ast {

NodeVisitor::NodeVisitor() = default;
NodeVisitor::~NodeVisitor() = default;

void NodeVisitor::Visit(const Node& node) {
  node.Accept(this);
}

#define V(name) \
  void name::Accept(NodeVisitor* visitor) const { visitor->Visit##name(*this); }
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

}  // namespace ast
}  // namespace joana

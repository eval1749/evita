// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/empty_node_visitor.h"

namespace joana {
namespace ast {

//
// EmptyNodeVisitor
//
EmptyNodeVisitor::EmptyNodeVisitor() = default;
EmptyNodeVisitor::~EmptyNodeVisitor() = default;

#define V(name) \
  void EmptyNodeVisitor::Visit##name(ast::name* node) {}
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

}  // namespace ast
}  // namespace joana

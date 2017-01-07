// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/ast/syntax_visitor.h"

#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/literals.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/types.h"

namespace joana {
namespace ast {

//
// SyntaxVisitor
//
SyntaxVisitor::SyntaxVisitor() = default;
SyntaxVisitor::~SyntaxVisitor() = default;

void SyntaxVisitor::Visit(const ast::Node& node) {
  node.syntax().Accept(this, node);
}

void SyntaxVisitor::VisitChildNodes(const ast::Node& node) {
  for (const auto& child : NodeTraversal::ChildNodesOf(node))
    Visit(child);
}

void SyntaxVisitor::VisitDefault(const ast::Node& node) {}

#define V(name)                                                            \
  void SyntaxVisitor::VisitInternal(const name& syntax,                    \
                                    const ast::Node& node) {               \
    VisitDefault(node);                                                    \
  }                                                                        \
                                                                           \
  void name::Accept(SyntaxVisitor* visitor, const ast::Node& node) const { \
    visitor->VisitInternal(*this, node);                                   \
  }

FOR_EACH_AST_SYNTAX(V)
#undef V

void DepthFirstTraverse(ast::SyntaxVisitor* visitor,
                        const ast::Node& start_node) {
  std::stack<std::pair<const ast::Node*, size_t>> stack;
  stack.push(std::make_pair(&start_node, 0));
  start_node.syntax().Accept(visitor, start_node);
  while (!stack.empty()) {
    const auto& container = *stack.top().first;
    const auto index = stack.top().second;
    if (index == container.arity()) {
      stack.pop();
      continue;
    }
    stack.top().second = index + 1;
    const auto& child = container.child_at(index);
    stack.push(std::make_pair(&child, 0));
    child.syntax().Accept(visitor, child);
  }
}

}  // namespace ast
}  // namespace joana

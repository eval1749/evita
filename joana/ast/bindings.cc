// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/bindings.h"

#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingCommaElement, 0)
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingInvalidElement, 0)
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingNameElement, 2)
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingProperty, 2)
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingRestElement, 1)

//
// ArrayBindingPatternSyntax
//
ArrayBindingPatternSyntax::ArrayBindingPatternSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::ArrayBindingPattern,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

ArrayBindingPatternSyntax::~ArrayBindingPatternSyntax() = default;

ChildNodes ArrayBindingPatternSyntax::ElementsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrayBindingPattern);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& ArrayBindingPatternSyntax::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrayBindingPattern);
  return node.child_at(0);
}

//
// ObjectBindingPatternSyntax
//
ObjectBindingPatternSyntax::ObjectBindingPatternSyntax()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::ObjectBindingPattern,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

ObjectBindingPatternSyntax::~ObjectBindingPatternSyntax() = default;

ChildNodes ObjectBindingPatternSyntax::ElementsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ObjectBindingPattern);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& ObjectBindingPatternSyntax::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ObjectBindingPattern);
  return node.child_at(0);
}

}  // namespace ast
}  // namespace joana

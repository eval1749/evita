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
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingProperty, 2)
IMPLEMENT_AST_SYNTAX_0(BidingElement, BindingRestElement, 1)

//
// ArrayBindingPattern
//
ArrayBindingPattern::ArrayBindingPattern()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::ArrayBindingPattern,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

ArrayBindingPattern::~ArrayBindingPattern() = default;

ChildNodes ArrayBindingPattern::ElementsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrayBindingPattern);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& ArrayBindingPattern::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrayBindingPattern);
  return node.child_at(0);
}

//
// BindingNameElement
//
BindingNameElement::BindingNameElement()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::BindingNameElement,
                     Format::Builder().set_arity(2).Build()) {}

BindingNameElement::~BindingNameElement() = default;

const Node& BindingNameElement::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BindingNameElement);
  return node.child_at(1);
}

const Node& BindingNameElement::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::BindingNameElement);
  return node.child_at(0);
}

//
// ObjectBindingPattern
//
ObjectBindingPattern::ObjectBindingPattern()
    : SyntaxTemplate(
          std::tuple<>(),
          SyntaxCode::ObjectBindingPattern,
          Format::Builder().set_arity(1).set_is_variadic(true).Build()) {}

ObjectBindingPattern::~ObjectBindingPattern() = default;

ChildNodes ObjectBindingPattern::ElementsOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ObjectBindingPattern);
  return ast::NodeTraversal::ChildNodesFrom(node, 1);
}

const Node& ObjectBindingPattern::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ObjectBindingPattern);
  return node.child_at(0);
}

}  // namespace ast
}  // namespace joana

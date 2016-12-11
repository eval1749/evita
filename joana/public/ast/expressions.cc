// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/expressions.h"

#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace ast {

//
// Expression
//
Expression::Expression(const SourceCodeRange& range) : ContainerNode(range) {}

Expression::~Expression() = default;

//
// InvalidExpression
//
InvalidExpression::InvalidExpression(const Node& node, int error_code)
    : Expression(node.range()), error_code_(error_code) {}

InvalidExpression::~InvalidExpression() = default;

//
// LiteralExpression
//
LiteralExpression::LiteralExpression(const Literal& literal)
    : Expression(literal.range()) {
  NodeEditor().AppendChild(this, const_cast<Literal*>(&literal));
}

LiteralExpression::~LiteralExpression() = default;

const Literal& LiteralExpression::literal() const {
  auto* const literal = NodeTraversal::FirstChildOf(*this);
  DCHECK(literal);
  return literal->As<Literal>();
}

//
// ReferenceExpression
//
ReferenceExpression::ReferenceExpression(const Name& name)
    : Expression(name.range()) {
  DCHECK(!name.IsKeyword()) << name;
  NodeEditor().AppendChild(this, const_cast<Name*>(&name));
}

ReferenceExpression::~ReferenceExpression() = default;

const Name& ReferenceExpression::name() const {
  auto* const child = NodeTraversal::FirstChildOf(*this);
  DCHECK(child);
  return child->As<Name>();
}

}  // namespace ast
}  // namespace joana

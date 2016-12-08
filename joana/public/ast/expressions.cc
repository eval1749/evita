// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/expressions.h"

#include "joana/public/ast/literals.h"

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
    : Expression(literal.range()), literal_(literal) {}

LiteralExpression::~LiteralExpression() = default;

}  // namespace ast
}  // namespace joana

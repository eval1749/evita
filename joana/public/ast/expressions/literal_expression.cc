// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/expressions/literal_expression.h"

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

LiteralExpression::LiteralExpression(const Literal& literal)
    : Expression(literal.location()), literal_(literal) {}

LiteralExpression::~LiteralExpression() = default;

}  // namespace ast
}  // namespace joana

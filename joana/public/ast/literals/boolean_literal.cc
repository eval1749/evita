// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/boolean_literal.h"

namespace joana {
namespace ast {

BooleanLiteral::BooleanLiteral(const SourceCodeRange& range, bool value)
    : Literal(range), value_(value) {}

BooleanLiteral::~BooleanLiteral() = default;

}  // namespace ast
}  // namespace joana

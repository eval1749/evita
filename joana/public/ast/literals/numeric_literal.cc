// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/numeric_literal.h"

namespace joana {
namespace ast {

NumericLiteral::NumericLiteral(const SourceCodeRange& location, double value)
    : Literal(location), value_(value) {}

NumericLiteral::~NumericLiteral() = default;

}  // namespace ast
}  // namespace joana

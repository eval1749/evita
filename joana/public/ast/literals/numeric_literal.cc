// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/numeric_literal.h"

namespace joana {
namespace ast {

NumericLiteral::NumericLiteral(const SourceCodeRange& range, double value)
    : Literal(range), value_(value) {}

NumericLiteral::~NumericLiteral() = default;

// Implements |Node| members
void NumericLiteral::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", " << value_;
}

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/public/ast/literals/string_literal.h"

#include "joana/public/escaped_string_piece.h"

namespace joana {
namespace ast {

StringLiteral::StringLiteral(const SourceCodeRange& range,
                             base::StringPiece16 data)
    : Literal(range), data_(base::string16(data.data(), data.size())) {}

StringLiteral::~StringLiteral() = default;

// Implements |Node| members
void StringLiteral::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", '" << EscapedStringPiece16(data_, '\'') << '\'';
}

}  // namespace ast
}  // namespace joana

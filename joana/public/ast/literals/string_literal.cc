// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/string_literal.h"

namespace joana {
namespace ast {

StringLiteral::StringLiteral(const SourceCodeRange& location,
                             base::StringPiece16 data)
    : Literal(location), data_(base::string16(data.data(), data.size())) {}

StringLiteral::~StringLiteral() = default;

}  // namespace ast
}  // namespace joana

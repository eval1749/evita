// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/null_literal.h"

namespace joana {
namespace ast {

NullLiteral::NullLiteral(const SourceCodeRange& location) : Literal(location) {}

NullLiteral::~NullLiteral() = default;

}  // namespace ast
}  // namespace joana

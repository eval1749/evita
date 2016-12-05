// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/literal.h"

namespace joana {
namespace ast {

Literal::Literal(const SourceCodeRange& location) : Node(location) {}

Literal::~Literal() = default;

}  // namespace ast
}  // namespace joana

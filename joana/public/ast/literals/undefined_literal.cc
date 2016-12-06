// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals/undefined_literal.h"

namespace joana {
namespace ast {

UndefinedLiteral::UndefinedLiteral(const SourceCodeRange& range)
    : Literal(range) {}

UndefinedLiteral::~UndefinedLiteral() = default;

}  // namespace ast
}  // namespace joana

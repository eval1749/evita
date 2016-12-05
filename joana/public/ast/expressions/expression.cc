// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/expressions/expression.h"

namespace joana {
namespace ast {

Expression::Expression(const SourceCodeRange& location)
    : ContainerNode(location) {}

Expression::~Expression() = default;

}  // namespace ast
}  // namespace joana

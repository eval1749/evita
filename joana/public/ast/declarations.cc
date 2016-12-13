// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/declarations.h"

#include "joana/public/ast/statements.h"

namespace joana {
namespace ast {

//
// ArrowFunction
//
ArrowFunction::ArrowFunction(const SourceCodeRange& range,
                             FunctionKind kind,
                             ExpressionList* parameters,
                             ArrowFunctionBody* body)
    : NodeTemplate(std::make_tuple(kind, parameters, body), range) {}

ArrowFunction::~ArrowFunction() = default;

//
// Declaration
//
Declaration::Declaration(const SourceCodeRange& range) : Node(range) {}

Declaration::~Declaration() = default;

}  // namespace ast
}  // namespace joana

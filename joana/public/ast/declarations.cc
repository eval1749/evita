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
                             Expression* parameter_list,
                             ArrowFunctionBody* body)
    : NodeTemplate(std::make_tuple(kind, parameter_list, body), range) {}

ArrowFunction::~ArrowFunction() = default;

//
// Declaration
//
Declaration::Declaration(const SourceCodeRange& range) : Node(range) {}

Declaration::~Declaration() = default;

//
// Function
//
Function::Function(const SourceCodeRange& range,
                   FunctionKind kind,
                   Token* name,
                   Expression* parameter_list,
                   Statement* body)
    : NodeTemplate(std::make_tuple(kind, name, parameter_list, body), range) {}

Function::~Function() = default;

//
// Method
//
Method::Method(const SourceCodeRange& range,
               FunctionKind kind,
               Expression* name,
               Expression* parameter_list,
               Statement* body)
    : NodeTemplate(std::make_tuple(kind, name, parameter_list, body), range) {}

Method::~Method() = default;

}  // namespace ast
}  // namespace joana

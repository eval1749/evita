// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/compilation_units.h"

namespace joana {
namespace ast {

//
// ExternsSyntax
//
ExternsSyntax::ExternsSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Externs,
                     Format::Builder().set_is_variadic(true).Build()) {}

ExternsSyntax::~ExternsSyntax() = default;

//
// ModuleSyntax
//
ModuleSyntax::ModuleSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Module,
                     Format::Builder().set_is_variadic(true).Build()) {}

ModuleSyntax::~ModuleSyntax() = default;

//
// ScriptSyntax
//
ScriptSyntax::ScriptSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Script,
                     Format::Builder().set_is_variadic(true).Build()) {}

ScriptSyntax::~ScriptSyntax() = default;

}  // namespace ast
}  // namespace joana

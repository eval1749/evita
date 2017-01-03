// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/compilation_units.h"

namespace joana {
namespace ast {

//
// CompilationUnit
//
CompilationUnit::CompilationUnit(SyntaxCode syntax_code)
    : SyntaxTemplate(std::tuple<>(),
                     syntax_code,
                     Format::Builder().set_is_variadic(true).Build()) {}

CompilationUnit::~CompilationUnit() = default;

//
// Externs
//
Externs::Externs() : CompilationUnit(SyntaxCode::Externs) {}

Externs::~Externs() = default;

//
// Module
//
Module::Module() : CompilationUnit(SyntaxCode::Module) {}

Module::~Module() = default;

//
// Script
//
Script::Script() : CompilationUnit(SyntaxCode::Script) {}

Script::~Script() = default;

}  // namespace ast
}  // namespace joana

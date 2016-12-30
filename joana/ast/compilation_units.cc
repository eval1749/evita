// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/compilation_units.h"

namespace joana {
namespace ast {

//
// CompilationUnit
//
CompilationUnit::CompilationUnit(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map)
    : NodeTemplate(&statements, range),
      jsdoc_map_(zone, jsdoc_map.begin(), jsdoc_map.end()) {}

CompilationUnit::~CompilationUnit() = default;

const JsDoc* CompilationUnit::JsDocFor(const ast::Node& node) const {
  const auto& it = jsdoc_map_.find(&node);
  return it == jsdoc_map_.end() ? nullptr : it->second;
}

//
// Externs
//
Externs::Externs(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map)
    : CompilationUnit(zone, range, statements, jsdoc_map) {}

Externs::~Externs() = default;

//
// Module
//
Module::Module(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map)
    : CompilationUnit(zone, range, statements, jsdoc_map) {}

Module::~Module() = default;

//
// Script
//
Script::Script(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& jsdoc_map)
    : CompilationUnit(zone, range, statements, jsdoc_map) {}

Script::~Script() = default;

}  // namespace ast
}  // namespace joana

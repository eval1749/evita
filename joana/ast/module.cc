// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/module.h"

namespace joana {
namespace ast {

//
// CompilationUnit
//
CompilationUnit::CompilationUnit(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& js_doc_map)
    : NodeTemplate(&statements, range),
      js_doc_map_(zone, js_doc_map.begin(), js_doc_map.end()) {}

CompilationUnit::~CompilationUnit() = default;

const JsDoc* CompilationUnit::JsDocFor(const ast::Node& node) const {
  const auto& it = js_doc_map_.find(&node);
  return it == js_doc_map_.end() ? nullptr : it->second;
}

//
// Module
//
Module::Module(
    Zone* zone,
    const SourceCodeRange& range,
    const StatementList& statements,
    const std::unordered_map<const Node*, const ast::JsDoc*>& js_doc_map)
    : CompilationUnit(zone, range, statements, js_doc_map) {}

Module::~Module() = default;

}  // namespace ast
}  // namespace joana

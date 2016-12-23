// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/module.h"

namespace joana {
namespace ast {

Module::Module(Zone* zone,
               const SourceCodeRange& range,
               const StatementList& statements,
               const std::unordered_map<const Node*, const ast::Annotation*>&
                   annotation_map)
    : NodeTemplate(&statements, range),
      annotation_map_(zone, annotation_map.begin(), annotation_map.end()) {}

Module::~Module() = default;

const Annotation* Module::AnnotationFor(const ast::Node& node) const {
  const auto& it = annotation_map_.find(&node);
  return it == annotation_map_.end() ? nullptr : it->second;
}

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type_map.h"

#include "base/logging.h"
#include "joana/analyzer/type.h"
#include "joana/ast/node.h"

namespace joana {
namespace analyzer {

//
// TypeMap
//
TypeMap::TypeMap() {}
TypeMap::~TypeMap() = default;

void TypeMap::RegisterType(const ast::Node& node, const Type& type) {
  const auto& result = type_map_.emplace(&node, &type);
  DCHECK(result.second) << "Node can have only one type " << node << std::endl
                        << type << std::endl
                        << *result.first->second;
}

const Type* TypeMap::TryTypeOf(const ast::Node& node) const {
  const auto& it = type_map_.find(&node);
  return it == type_map_.end() ? nullptr : it->second;
}

const Type& TypeMap::TypeOf(const ast::Node& node) const {
  if (auto* present = TryTypeOf(node))
    return *present;
  NOTREACHED() << "No type for " << node;
  return *static_cast<const Type*>(nullptr);
}

}  // namespace analyzer
}  // namespace joana

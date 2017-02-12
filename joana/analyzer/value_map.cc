// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/value_map.h"

#include "base/logging.h"
#include "joana/analyzer/values.h"
#include "joana/ast/declarations.h"

namespace joana {
namespace analyzer {

//
// ValueMap
//
ValueMap::ValueMap() {}
ValueMap::~ValueMap() = default;

const Value& ValueMap::RegisterValue(const ast::Node& node,
                                     const Value& value) {
#if DCHECK_IS_ON()
  if (node.Is<ast::Class>()) {
    DCHECK(value.Is<Class>()) << node << ' ' << value;
  } else if (node.Is<ast::Function>()) {
    DCHECK(value.Is<Class>() || value.Is<Function>()) << node << ' ' << value;
  } else if (node.Is<ast::Method>()) {
    DCHECK(value.Is<Function>()) << node << ' ' << value;
  }
#endif
  const auto& result = value_map_.emplace(&node, &value);
  DCHECK(result.second) << "Node can have only one value " << node << std::endl
                        << value << std::endl
                        << *result.first->second;
  return value;
}

const Value* ValueMap::TryValueOf(const ast::Node& node) const {
  const auto& it = value_map_.find(&node);
  return it == value_map_.end() ? nullptr : it->second;
}

const Value& ValueMap::ValueOf(const ast::Node& node) const {
  if (auto* present = TryValueOf(node))
    return *present;
  NOTREACHED() << "No value for " << node;
  return *static_cast<Value*>(nullptr);
}

}  // namespace analyzer
}  // namespace joana

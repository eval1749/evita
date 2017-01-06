// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/value_map.h"

#include "base/logging.h"
#include "joana/analyzer/value.h"
#include "joana/ast/node.h"

namespace joana {
namespace analyzer {

//
// ValueMap
//
ValueMap::ValueMap() {}
ValueMap::~ValueMap() = default;

Value& ValueMap::RegisterValue(const ast::Node& node, Value* value) {
  const auto& result = value_map_.emplace(&node, value);
  DCHECK(result.second) << "Node can have only one value " << node << std::endl
                        << *value << std::endl
                        << *result.first->second;
  return *value;
}

Value* ValueMap::TryValueOf(const ast::Node& node) const {
  const auto& it = value_map_.find(&node);
  return it == value_map_.end() ? nullptr : it->second;
}

Value& ValueMap::ValueOf(const ast::Node& node) const {
  if (auto* present = TryValueOf(node))
    return *present;
  NOTREACHED() << "No value for " << node;
  return *static_cast<Value*>(nullptr);
}

}  // namespace analyzer
}  // namespace joana

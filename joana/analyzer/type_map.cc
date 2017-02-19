// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type_map.h"

#include "base/logging.h"
#include "joana/analyzer/types.h"
#include "joana/ast/declarations.h"

namespace joana {
namespace analyzer {

//
// TypeMap
//
TypeMap::TypeMap() {}
TypeMap::~TypeMap() = default;

void TypeMap::RegisterType(const ast::Node& node, const Type& type) {
#if DCHECK_IS_ON()
  if (node.Is<ast::Class>()) {
    DCHECK(type.Is<ClassType>()) << node << ' ' << type;
  } else if (node.Is<ast::Function>()) {
    DCHECK(type.Is<ClassType>() || type.Is<FunctionType>()) << node << ' '
                                                            << type;
  } else if (node.Is<ast::Method>() &&
             ast::Method::FunctionKindOf(node) != ast::FunctionKind::Getter) {
    DCHECK(type.Is<FunctionType>()) << node << ' ' << type;
  }
#endif
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

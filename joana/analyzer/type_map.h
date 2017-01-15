// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_MAP_H_
#define JOANA_ANALYZER_TYPE_MAP_H_

#include <unordered_map>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Type;

//
// TypeMap
//
class TypeMap final {
 public:
  TypeMap();
  ~TypeMap();

  void RegisterType(const ast::Node& node, const Type& type);
  const Type* TryTypeOf(const ast::Node& node) const;
  const Type& TypeOf(const ast::Node& node) const;

 private:
  friend class NameResolver;

  std::unordered_map<const ast::Node*, const Type*> type_map_;

  DISALLOW_COPY_AND_ASSIGN(TypeMap);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_MAP_H_

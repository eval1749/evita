// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUE_MAP_H_
#define JOANA_ANALYZER_VALUE_MAP_H_

#include <unordered_map>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Value;

//
// ValueMap
//
class ValueMap final {
 public:
  ValueMap();
  ~ValueMap();

  Value& RegisterValue(const ast::Node& node, Value* value);
  Value* TryValueOf(const ast::Node& node) const;
  Value& ValueOf(const ast::Node& node) const;

 private:
  friend class NameResolver;

  std::unordered_map<const ast::Node*, Value*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(ValueMap);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUE_MAP_H_

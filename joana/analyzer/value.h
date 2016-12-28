// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUE_H_
#define JOANA_ANALYZER_VALUE_H_

#include "base/macros.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"

namespace joana {
namespace ast {
class Node;
}
namespace analyzer {

#define DECLARE_ANALYZE_VALUE(name, base) \
  DECLARE_CASTABLE_CLASS(name, base);     \
  friend class Factory;

#define DECLARE_ABSTRACT_ANALYZE_VALUE(name, base) \
  DECLARE_ANALYZE_VALUE(name, base)

#define DECLARE_CONCRETE_ANALYZE_VALUE(name, base) \
  DECLARE_ANALYZE_VALUE(name, base)

//
// Value
//
class Value : public Castable<Value>, public ZoneAllocated {
  DECLARE_ABSTRACT_ANALYZE_VALUE(Value, Castable);

 public:
  virtual ~Value();

  // Return the AST node which associated to this value.
  const ast::Node& node() const { return node_; }

 protected:
  explicit Value(const ast::Node& node);

 private:
  friend class Factory;

  const ast::Node& node_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUE_H_

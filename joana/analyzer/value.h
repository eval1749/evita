// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUE_H_
#define JOANA_ANALYZER_VALUE_H_

#include <iosfwd>

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
  friend class Editor;                    \
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
  class Editor;

  virtual ~Value();

  // Returns unique identifier of |Value|.
  int id() const { return id_; }

  // Return the AST node which associated to this value.
  const ast::Node& node() const { return node_; }

 protected:
  Value(int id, const ast::Node& node);

 private:
  friend class Factory;

  const int id_;
  const ast::Node& node_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

// See "value_printer.cc" for implementation of |Value| printer.
std::ostream& operator<<(std::ostream& ostream, const Value& value);
std::ostream& operator<<(std::ostream& ostream, const Value* value);

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUE_H_

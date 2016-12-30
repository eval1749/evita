// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_H_
#define JOANA_ANALYZER_ENVIRONMENT_H_

#include "base/macros.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/memory/zone_unordered_map.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {
namespace ast {
class Name;
class Node;
}
namespace analyzer {

class Value;

//
// Environment
//
class Environment final : public ZoneAllocated {
 public:
  class Builder;

  virtual ~Environment();

  const ZoneVector<const ast::Name*>& names() const { return names_; }

  // Returns outer environment of this environment, or returns null If this
  // environment is *global* environment.
  Environment* outer() const { return outer_; }

  // Return the AST node which associated to this Environment.
  const ast::Node& owner() const { return owner_; }

  Value* BindingOf(const ast::Name& name) const;

 private:
  friend class EnvironmentBuilder;
  friend class Factory;

  Environment(Zone* zone, Environment* outer, const ast::Node& owner);
  Environment(Zone* zone, const ast::Node& owner);

  void Bind(const ast::Name& name, Value* value);

  ZoneVector<const ast::Name*> names_;
  ZoneUnorderedMap<int, const ast::Name*> name_map_;
  Environment* const outer_;
  const ast::Node& owner_;
  ZoneUnorderedMap<int, Value*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Environment);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_H_

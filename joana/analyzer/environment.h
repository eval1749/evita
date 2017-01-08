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
class Node;
}
namespace analyzer {

class Type;
class Variable;

//
// Environment
//
class Environment final : public ZoneAllocated {
 public:
  class Builder;

  virtual ~Environment();

  const ZoneVector<const ast::Node*>& names_for_testing() const {
    return names_;
  }

  // Returns outer environment of this environment, or returns null If this
  // environment is *global* environment.
  Environment* outer() const { return outer_; }

  // Return the AST node which associated to this Environment.
  const ast::Node& owner() const { return owner_; }

  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

 private:
  friend class Context;
  friend class EnvironmentBuilder;

  Environment(Zone* zone, Environment* outer, const ast::Node& owner);
  Environment(Zone* zone, const ast::Node& owner);

  void BindVariable(const ast::Node& name, Variable* value);
  void BindType(const ast::Node& name, const Type& type);

  ZoneVector<const ast::Node*> names_;
  ZoneUnorderedMap<int, const ast::Node*> name_map_;
  Environment* const outer_;
  const ast::Node& owner_;
  ZoneUnorderedMap<int, const Type*> type_map_;
  ZoneUnorderedMap<int, Variable*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Environment);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_H_

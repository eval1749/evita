// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_H_
#define JOANA_ANALYZER_ENVIRONMENT_H_

#include <unordered_map>

#include "base/macros.h"
#include "joana/base/memory/zone_allocated.h"

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

  // Return the AST node which associated to this Environment.
  const ast::Node& owner() const { return owner_; }

  Value* BindingOf(const ast::Name& name) const;

 private:
  friend class EnvironmentBuilder;
  friend class Factory;

  Environment(Environment* outer, const ast::Node& owner);
  explicit Environment(const ast::Node& owner);

  void Bind(const ast::Name& name, Value* value);

  std::unordered_map<int, const ast::Name*> name_map_;
  Environment* const outer_;
  const ast::Node& owner_;
  std::unordered_map<int, Value*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Environment);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_H_

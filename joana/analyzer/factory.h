// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_FACTORY_H_
#define JOANA_ANALYZER_FACTORY_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/base/memory/zone.h"

namespace joana {
namespace ast {
class Node;
}
namespace analyzer {

class Environment;
class Value;

//
// Factory
//
class Factory final {
 public:
  // |zone| A zone to store analyze results.
  explicit Factory(Zone* zone);
  ~Factory();

  Environment& global_environment() const { return global_environment_; }

  // Query members
  Environment& EnvironmentOf(const ast::Node& node) const;
  Value& ValueOf(const ast::Node& node) const;

  // Registration
  Value& RegisterValue(const ast::Node& node, Value* value);

  // Factory members
  Environment& NewEnvironment(Environment* outer, const ast::Node& owner);
  Value& NewFunction(const ast::Node& node);
  Value& NewProperty(const ast::Node& node);
  Value& NewVariable(const ast::Node& assignment, const ast::Node& name);

 private:
  static Environment& NewGlobalEnvironment(Zone* zone);

  int NextValueId();

  int current_value_id_ = 0;
  std::unordered_map<const ast::Node*, Environment*> environment_map_;
  Environment& global_environment_;
  std::unordered_map<const ast::Node*, Value*> value_map_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_FACTORY_H_

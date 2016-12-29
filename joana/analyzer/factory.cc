// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/factory.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// Factory
//
Factory::Factory(Zone* zone)
    : global_environment_(NewGlobalEnvironment(zone)), zone_(*zone) {}

Factory::~Factory() = default;

// Factory members
Environment& Factory::NewEnvironment(Environment* outer,
                                     const ast::Node& owner) {
  return *new (&zone_) Environment(&zone_, outer, owner);
}

Value& Factory::NewFunction(const ast::Node& node) {
  return *new (&zone_) Function(&zone_, node);
}

Value& Factory::NewProperty(const ast::Node& node) {
  return *new (&zone_) Property(&zone_, node);
}

Value& Factory::NewVariable(const ast::Node& assignment,
                            const ast::Node& name) {
  return *new (&zone_) Variable(&zone_, assignment, name);
}

// static
Environment& Factory::NewGlobalEnvironment(Zone* zone) {
  const auto& module = BuiltInWorld::GetInstance()->global_module();
  return *new (zone) Environment(zone, module);
}

}  // namespace analyzer
}  // namespace joana

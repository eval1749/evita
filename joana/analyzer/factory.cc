// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/factory.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/properties.h"
#include "joana/analyzer/values.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Factory
//
Factory::Factory(Zone* zone)
    : global_environment_(NewGlobalEnvironment(zone)), zone_(*zone) {}

Factory::~Factory() = default;

// Query members
Environment& Factory::EnvironmentOf(const ast::Node& node) const {
  const auto& it = environment_map_.find(&node);
  if (it != environment_map_.end())
    return *it->second;
  DCHECK(node.syntax().Is<ast::CompilationUnit>()) << node;
  return global_environment();
}

Value* Factory::TryValueOf(const ast::Node& node) const {
  const auto& it = value_map_.find(&node);
  return it == value_map_.end() ? nullptr : it->second;
}

Value& Factory::ValueOf(const ast::Node& node) const {
  if (auto* present = TryValueOf(node))
    return *present;
  NOTREACHED() << "No value for " << node;
  return const_cast<Factory*>(this)->NewUndefined(node);
}

// Factory members
Property& Factory::GetOrNewProperty(Properties* properties,
                                    const ast::Node& node) {
  if (auto* present = properties->TryGet(node))
    return *present;
  return properties->Add(&NewProperty(node));
}

Environment& Factory::NewEnvironment(Environment* outer,
                                     const ast::Node& owner) {
  auto& environment = *new (&zone_) Environment(&zone_, outer, owner);
  const auto& result = environment_map_.emplace(&owner, &environment);
  DCHECK(result.second) << "Node can have only one environment " << owner;
  return environment;
}

Function& Factory::NewFunction(const ast::Node& node) {
  auto& properties = NewProperties(node);
  return *new (&zone_) Function(NextValueId(), node, &properties);
}

Value& Factory::NewOrdinaryObject(const ast::Node& node) {
  auto& properties = NewProperties(node);
  return RegisterValue(
      node, new (&zone_) OrdinaryObject(NextValueId(), node, &properties));
}

Properties& Factory::NewProperties(const ast::Node& owner) {
  return *new (&zone_) Properties(&zone_, owner);
}

Property& Factory::NewProperty(const ast::Node& key) {
  auto& properties = NewProperties(key);
  return *new (&zone_) Property(&zone_, NextValueId(), key, &properties);
}

Value& Factory::NewUndefined(const ast::Node& node) {
  return RegisterValue(node, new (&zone_) Undefined(NextValueId(), node));
}

Variable& Factory::NewVariable(const ast::Node& origin, const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  auto& properties = NewProperties(name);
  return *new (&zone_)
      Variable(&zone_, NextValueId(), origin, name, &properties);
}

// static
Environment& Factory::NewGlobalEnvironment(Zone* zone) {
  const auto& module = BuiltInWorld::GetInstance()->global_module();
  return *new (zone) Environment(zone, module);
}

int Factory::NextValueId() {
  return ++current_value_id_;
}

Value& Factory::RegisterValue(const ast::Node& node, Value* value) {
  const auto& result = value_map_.emplace(&node, value);
  DCHECK(result.second) << "Node can have only one value " << node << std::endl
                        << *value << std::endl
                        << *result.first->second;
  return *value;
}

}  // namespace analyzer
}  // namespace joana

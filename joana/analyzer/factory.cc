// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/factory.h"

#include "joana/analyzer/properties.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Factory
//
Factory::Factory(Zone* zone) : zone_(*zone) {}

Factory::~Factory() = default;

// Factory members
Property& Factory::GetOrNewProperty(Properties* properties,
                                    const ast::Node& node) {
  if (auto* present = properties->TryGet(node))
    return *present;
  return properties->Add(&NewProperty(node));
}

Function& Factory::NewFunction(const ast::Node& node) {
  auto& properties = NewProperties(node);
  return *new (&zone_) Function(NextValueId(), node, &properties);
}

Value& Factory::NewOrdinaryObject(const ast::Node& node) {
  auto& properties = NewProperties(node);
  return *new (&zone_) OrdinaryObject(NextValueId(), node, &properties);
}

Properties& Factory::NewProperties(const ast::Node& owner) {
  return *new (&zone_) Properties(&zone_, owner);
}

Property& Factory::NewProperty(const ast::Node& key) {
  auto& properties = NewProperties(key);
  return *new (&zone_) Property(&zone_, NextValueId(), key, &properties);
}

Value& Factory::NewUndefined(const ast::Node& node) {
  return *new (&zone_) Undefined(NextValueId(), node);
}

Variable& Factory::NewVariable(const ast::Node& origin, const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  auto& properties = NewProperties(name);
  return *new (&zone_)
      Variable(&zone_, NextValueId(), origin, name, &properties);
}

int Factory::NextValueId() {
  return ++current_value_id_;
}

}  // namespace analyzer
}  // namespace joana

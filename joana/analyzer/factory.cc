// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/factory.h"

#include "joana/analyzer/environment.h"
#include "joana/analyzer/properties.h"
#include "joana/analyzer/value_map.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Factory
//
Factory::Factory(Zone* zone) : value_map_(new ValueMap()), zone_(*zone) {}

Factory::~Factory() = default;

// Query members
Value* Factory::TryValueOf(const ast::Node& node) const {
  return value_map_->TryValueOf(node);
}

Value& Factory::ValueOf(const ast::Node& node) const {
  return value_map_->ValueOf(node);
}

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

Value& Factory::RegisterValue(const ast::Node& node, Value* value) {
  return value_map_->RegisterValue(node, value);
}

}  // namespace analyzer
}  // namespace joana

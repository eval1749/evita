// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <tuple>

#include "joana/analyzer/factory.h"

#include "joana/analyzer/properties.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

template <typename T>
size_t SizeOf(size_t number_of_elements) {
  return sizeof(T) - sizeof(Type*) + sizeof(Type*) * number_of_elements;
}

}  // namespace

//
// Factory::Cache
//
class Factory::Cache final {
 public:
  Cache();
  ~Cache();

  template <typename Key>
  Value* Find(const Key& key) {
    const auto& map = MapFor(key);
    const auto& it = map.find(key);
    return it == map.end() ? nullptr : it->second;
  }

  template <typename Key>
  void Register(const Key& key, Value* value) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, value);
    DCHECK(result.second);
  }

 private:
  using ConstructedClassKey = std::tuple<Class*, std::vector<const Type*>>;
  using ConstructedClassMap = std::map<ConstructedClassKey, Value*>;

  ConstructedClassMap& MapFor(const ConstructedClassKey&) {
    return constructed_class_map_;
  }

  ConstructedClassMap constructed_class_map_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

Factory::Cache::Cache() = default;
Factory::Cache::~Cache() = default;

//
// Factory
//
Factory::Factory(Zone* zone) : cache_(new Cache()), zone_(*zone) {}

Factory::~Factory() = default;

// Factory members
Class& Factory::NewClass(const ast::Node& node,
                         ClassKind kind,
                         const std::vector<const TypeParameter*>& parameters) {
  return *new (&zone_) Class(&zone_, NextValueId(), node, kind, parameters);
}

Value& Factory::NewConstructedClass(Class* generic_class,
                                    const std::vector<const Type*> arguments) {
  const auto& key = std::make_tuple(generic_class, arguments);
  if (auto* present = cache_->Find(key))
    return *present;
  const auto size = SizeOf<ConstructedClass>(arguments.size());
  auto& new_value = *new (zone_.Allocate(size)) ConstructedClass(
      NextValueId(), generic_class, arguments);
  cache_->Register(key, &new_value);
  return new_value;
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

Property& Factory::NewProperty(Visibility visibility, const ast::Node& key) {
  auto& properties = NewProperties(key);
  return *new (&zone_)
      Property(&zone_, NextValueId(), visibility, key, &properties);
}

Value& Factory::NewUndefined(const ast::Node& node) {
  return *new (&zone_) Undefined(NextValueId(), node);
}

Variable& Factory::NewVariable(VariableKind kind, const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  auto& properties = NewProperties(name);
  return *new (&zone_) Variable(&zone_, NextValueId(), kind, name, &properties);
}

int Factory::NextValueId() {
  return ++current_value_id_;
}

void Factory::ResetValueId() {
  const auto kValueIdStart = 1000;
  DCHECK_LT(current_value_id_, kValueIdStart);
  current_value_id_ = kValueIdStart;
}

}  // namespace analyzer
}  // namespace joana

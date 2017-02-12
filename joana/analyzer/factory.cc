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
  using ConstructedClassKey =
      std::tuple<const Class*, std::vector<const Type*>>;
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
const Class& Factory::NewConstructedClass(
    const GenericClass& generic_class,
    const std::vector<const Type*>& arguments) {
  const auto& key = std::make_tuple(&generic_class, arguments);
  if (auto* present = cache_->Find(key))
    return present->As<ConstructedClass>();
  const auto size = SizeOf<ConstructedClass>(arguments.size());
  auto& new_value = *new (zone_.Allocate(size)) ConstructedClass(
      &zone_, NextValueId(), generic_class, arguments);
  cache_->Register(key, &new_value);
  return new_value;
}

const Function& Factory::NewFunction(const ast::Node& name,
                                     const ast::Node& node) {
  DCHECK(CanBeValueName(name)) << name;
  auto& properties = NewProperties(node);
  return *new (&zone_) Function(NextValueId(), name, node, &properties);
}

const Class& Factory::NewGenericClass(
    ClassKind kind,
    const ast::Node& name,
    const ast::Node& node,
    const std::vector<const TypeParameter*>& parameters,
    Properties* properties) {
  const auto size = SizeOf<GenericClass>(parameters.size());
  return *new (zone_.Allocate(size)) GenericClass(
      &zone_, NextValueId(), kind, name, node, parameters, properties);
}

const Class& Factory::NewNormalClass(ClassKind kind,
                                     const ast::Node& name,
                                     const ast::Node& node,
                                     Properties* properties) {
  return *new (&zone_)
      NormalClass(&zone_, NextValueId(), kind, name, node, properties);
}

const Value& Factory::NewOrdinaryObject(const ast::Node& node) {
  auto& properties = NewProperties(node);
  return *new (&zone_) OrdinaryObject(NextValueId(), node, &properties);
}

Properties& Factory::NewProperties(const ast::Node& owner) {
  return *new (&zone_) Properties(&zone_, owner);
}

const Property& Factory::NewProperty(Visibility visibility,
                                     const ast::Node& key,
                                     ValueHolderData* data,
                                     Properties* properties) {
  return *new (&zone_)
      Property(NextValueId(), visibility, key, data, properties);
}

const Value& Factory::NewUndefined(const ast::Node& node) {
  return *new (&zone_) Undefined(NextValueId(), node);
}

ValueHolderData& Factory::NewValueHolderData() {
  return *new (&zone_) ValueHolderData(&zone_);
}

const Variable& Factory::NewVariable(VariableKind kind,
                                     const ast::Node& name,
                                     ValueHolderData* data,
                                     Properties* properties) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return *new (&zone_) Variable(NextValueId(), kind, name, data, properties);
}

int Factory::NextValueId() {
  return ++current_value_id_;
}

void Factory::ResetCurrentId() {
  const auto kValueIdStart = 1000;
  DCHECK_LT(current_value_id_, kValueIdStart);
  current_value_id_ = kValueIdStart;
}

void Factory::ResetCurrentIdForTesting(int current_id) {
  DCHECK_GT(current_id, current_value_id_);
  current_value_id_ = current_id;
}

}  // namespace analyzer
}  // namespace joana

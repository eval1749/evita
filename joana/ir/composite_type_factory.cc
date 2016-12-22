// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <tuple>

#include "joana/ir/composite_type_factory.h"

#include "joana/ir/composite_types.h"

namespace joana {
namespace ir {

namespace {

enum class CompositeTypeId {
#define V(name, ...) name,
  FOR_EACH_IR_COMPOSITE_TYPE(V)
#undef V
};

}  // namespace

//
// CompositeTypeFactory::Cache
//
class CompositeTypeFactory::Cache final {
 public:
  Cache();
  ~Cache();

  template <typename Key>
  const Type* Find(const Key& key) {
    const auto& map = MapFor(key);
    const auto& it = map.find(key);
    return it == map.end() ? nullptr : it->second;
  }

  template <typename Key>
  const Type& Register(const Key& key, const Type& type) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, &type);
    DCHECK(result.second);
    return *result.first->second;
  }

 private:
  using Key1 = std::tuple<CompositeTypeId, const Type*>;
  using Key2 = std::tuple<CompositeTypeId, const Type*, const Type*>;
  using KeyN = std::vector<const Type*>;

  std::map<Key1, const Type*>& MapFor(const Key1&) { return map_1_; }
  std::map<Key2, const Type*>& MapFor(const Key2&) { return map_2_; }
  std::map<KeyN, const Type*>& MapFor(const KeyN&) { return map_n_; }

  std::map<Key1, const Type*> map_1_;
  std::map<Key2, const Type*> map_2_;
  std::map<KeyN, const Type*> map_n_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

CompositeTypeFactory::Cache::Cache() = default;
CompositeTypeFactory::Cache::~Cache() = default;

//
// CompositeTypeFactory
//
CompositeTypeFactory::CompositeTypeFactory(Zone* zone)
    : cache_(new Cache()), zone_(*zone) {}

CompositeTypeFactory::~CompositeTypeFactory() = default;

const Type& CompositeTypeFactory::NewFunctionType(const Type& parameters_type,
                                                  const Type& return_type) {
  const auto& key = std::make_tuple(CompositeTypeId::Function, &parameters_type,
                                    &return_type);
  if (const auto* present = cache_->Find(key))
    return *present;
  return cache_->Register(
      key,
      *new (&zone_) FunctionType(parameters_type.As<TupleType>(), return_type));
}

const Type& CompositeTypeFactory::NewReferenceType(const Type& to) {
  const auto& key = std::make_tuple(CompositeTypeId::Reference, &to);
  if (const auto* present = cache_->Find(key))
    return *present;
  return cache_->Register(key, *new (&zone_) ReferenceType(to));
}

const Type& CompositeTypeFactory::NewTupleType(
    const std::vector<const Type*>& members) {
  if (const auto* present = cache_->Find(members))
    return *present;
  return cache_->Register(members, *new (&zone_) TupleType(&zone_, members));
}

const Type& CompositeTypeFactory::NewUnionType(
    const std::vector<const Type*>& members) {
  if (const auto* present = cache_->Find(members))
    return *present;
  return cache_->Register(members, *new (&zone_) UnionType(&zone_, members));
}

}  // namespace ir
}  // namespace joana

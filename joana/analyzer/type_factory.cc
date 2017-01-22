// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "joana/analyzer/type_factory.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

namespace {

struct LabelName {
  const ast::Node* name;

  bool operator<(const LabelName& other) const {
    return ast::Name::KindOf(*name) < ast::Name::KindOf(*other.name);
  }
};

template <typename T>
size_t SizeOf(size_t number_of_elements) {
  return sizeof(T) - sizeof(Type*) + sizeof(Type*) * number_of_elements;
}

}  // namespace

//
// TypeFactory::Cache
//
class TypeFactory::Cache final {
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
  void Register(const Key& key, const Type& type) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, &type);
    DCHECK(result.second);
  }

 private:
  using ClassTypeKey = Value*;
  using ClassTypeMap = std::unordered_map<ClassTypeKey, const Type*>;
  using FunctionTypeKey = std::tuple<FunctionTypeKind,
                                     std::vector<const TypeParameter*>,
                                     FunctionTypeArity,
                                     std::vector<const Type*>,
                                     const Type*,
                                     const Type*>;
  using FunctionTypeMap = std::map<FunctionTypeKey, const Type*>;
  using LabeledTypeKey = std::pair<LabelName, const Type*>;
  using LabeledTypeMap = std::map<LabeledTypeKey, const Type*>;
  using PrimitiveTypeMap = std::unordered_map<ast::TokenKind, const Type*>;
  using RecordTypeKey = std::vector<const LabeledType*>;
  using RecordTypeMap = std::map<RecordTypeKey, const Type*>;
  using TupleTypeKey = std::vector<const Type*>;
  using TupleTypeMap = std::map<TupleTypeKey, const Type*>;
  using UnionTypeKey = std::set<const Type*>;
  using UnionTypeMap = std::map<UnionTypeKey, const Type*>;

  ClassTypeMap& MapFor(const ClassTypeKey& class_value) {
    return class_type_map_;
  }
  FunctionTypeMap& MapFor(const FunctionTypeKey&) { return function_type_map_; }
  LabeledTypeMap& MapFor(const LabeledTypeKey&) { return labeled_type_map_; }
  PrimitiveTypeMap& MapFor(ast::TokenKind kind) { return primitive_type_map_; }
  RecordTypeMap& MapFor(const RecordTypeKey&) { return record_type_map_; }
  TupleTypeMap& MapFor(const TupleTypeKey&) { return tuple_type_map_; }
  UnionTypeMap& MapFor(const UnionTypeKey&) { return union_type_map_; }

  ClassTypeMap class_type_map_;
  FunctionTypeMap function_type_map_;
  LabeledTypeMap labeled_type_map_;
  PrimitiveTypeMap primitive_type_map_;
  RecordTypeMap record_type_map_;
  TupleTypeMap tuple_type_map_;
  UnionTypeMap union_type_map_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

TypeFactory::Cache::Cache() = default;
TypeFactory::Cache::~Cache() = default;

//
// TypeFactory
//
TypeFactory::TypeFactory(Zone* zone)
    : cache_(new Cache()),
      zone_(*zone),
      any_type_(*new (zone) AnyType(NextTypeId())),
      invalid_type_(*new (zone) InvalidType(NextTypeId())),
      nil_type_(*new (zone) NilType(NextTypeId())),
      null_type_(*new (zone) NullType(NextTypeId())),
      unspecified_type_(*new (zone) UnspecifiedType(NextTypeId())),
      void_type_(*new (zone) VoidType(NextTypeId())) {
  current_type_id_ = 100;
  InstallPrimitiveTypes();
  current_type_id_ = 1000;
}

TypeFactory::~TypeFactory() = default;

void TypeFactory::InstallPrimitiveTypes() {
  for (const auto id : BuiltInWorld::GetInstance()->primitive_types()) {
    const auto& name = BuiltInWorld::GetInstance()->NameOf(id);
    const auto& type = *new (&zone_) PrimitiveType(NextTypeId(), name);
    cache_->Register(ast::Name::KindOf(name), type);
  }
}

int TypeFactory::NextTypeId() {
  return ++current_type_id_;
}

const Type& TypeFactory::NewClassType(Value* class_value) {
  DCHECK(class_value->Is<Class>() || class_value->Is<ConstructedClass>())
      << class_value;
  const auto* type = cache_->Find(class_value);
  if (type)
    return *type;
  const auto& new_type = *new (&zone_) ClassType(NextTypeId(), class_value);
  cache_->Register(class_value, new_type);
  return new_type;
}

const Type& TypeFactory::NewFunctionType(
    FunctionTypeKind kind,
    const std::vector<const TypeParameter*>& type_parameters,
    const FunctionTypeArity& arity,
    const std::vector<const Type*>& parameters,
    const Type& return_type,
    const Type& this_type) {
  const auto& key = std::make_tuple(kind, type_parameters, arity, parameters,
                                    &return_type, &this_type);
  const auto* present = cache_->Find(key);
  if (present)
    return *present;
  const auto size =
      SizeOf<FunctionType>(type_parameters.size() + parameters.size());
  const auto& new_type = *new (zone_.Allocate(size)) FunctionType(
      NextTypeId(), kind, type_parameters, arity, parameters, return_type,
      this_type);
  cache_->Register(key, new_type);
  return new_type;
}

const Type& TypeFactory::NewLabeledType(const ast::Node& name,
                                        const Type& type) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  DCHECK(!type.Is<LabeledType>()) << name << ':' << type;
  const auto& key = std::make_pair(LabelName{&name}, &type);
  const auto* present = cache_->Find(key);
  if (present)
    return *present;
  const auto& new_type = *new (&zone_) LabeledType(NextTypeId(), name, type);
  cache_->Register(key, new_type);
  return new_type;
}

const Type& TypeFactory::NewPrimitiveType(ast::TokenKind id) {
  return *cache_->Find(id);
}

const Type& TypeFactory::NewRecordType(
    const std::vector<const LabeledType*>& members) {
  const auto* present = cache_->Find(members);
  if (present)
    return *present;
  const auto size = SizeOf<RecordType>(members.size());
  const auto& new_type =
      *new (zone_.Allocate(size)) RecordType(NextTypeId(), members);
  cache_->Register(members, new_type);
  return new_type;
}

const Type& TypeFactory::NewTupleTypeFromVector(
    const std::vector<const Type*>& members) {
  const auto* type = cache_->Find(members);
  if (type)
    return *type;
  const auto size = SizeOf<TupleType>(members.size());
  const auto& new_type =
      *new (zone_.Allocate(size)) TupleType(NextTypeId(), members);
  cache_->Register(members, new_type);
  return new_type;
}

const Type& TypeFactory::NewTypeAlias(const ast::Node& name,
                                      const ast::Node& type) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  DCHECK(type.syntax().Is<ast::Type>()) << name << ' ' << type;
  return *new (&zone_) TypeAlias(NextTypeId(), name, type);
}

const Type& TypeFactory::NewTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return *new (&zone_) TypeName(NextTypeId(), name);
}

const Type& TypeFactory::NewTypeParameter(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return *new (&zone_) TypeParameter(NextTypeId(), name);
}

const Type& TypeFactory::NewUnionTypeFromVector(
    const std::vector<const Type*>& passed_members) {
  std::set<const Type*> members;
  // TODO(eval1749): We should omit subtype members.
  for (const auto& member : passed_members) {
    if (member->Is<AnyType>())
      return *member;
    if (member->Is<NilType>())
      continue;
    if (member->Is<UnionType>()) {
      for (const auto& member2 : member->As<UnionType>().members())
        members.emplace(&member2);
      continue;
    }
    members.emplace(member);
  }
  if (members.empty())
    return nil_type();
  if (members.size() == 1)
    return **members.begin();
  const auto* type = cache_->Find(members);
  if (type)
    return *type;
  const auto size = SizeOf<UnionType>(members.size());
  const auto& new_type =
      *new (zone_.Allocate(size)) UnionType(NextTypeId(), members);
  cache_->Register(members, new_type);
  return new_type;
}

}  // namespace analyzer
}  // namespace joana

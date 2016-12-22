// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>

#include "joana/ir/type_factory.h"

#include "joana/ir/composite_type_factory.h"
#include "joana/ir/composite_types.h"
#include "joana/ir/primitive_type_factory.h"
#include "joana/ir/primitive_types.h"

namespace joana {
namespace ir {

//
// TypeFactory
//
TypeFactory::TypeFactory(Zone* zone)
    : composite_type_factory_(new CompositeTypeFactory(zone)) {}
TypeFactory::~TypeFactory() = default;

#define V(capital, underscore)                                       \
  const Type& TypeFactory::underscore##_type() const {               \
    return PrimitiveTypeFactory::GetInstance()->underscore##_type(); \
  }
FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V

const Type& TypeFactory::NewFunctionType(const Type& parameters_type,
                                         const Type& return_type) {
  return composite_type_factory_->NewFunctionType(parameters_type, return_type);
}

const Type& TypeFactory::NewReferenceType(const Type& to) {
  return composite_type_factory_->NewReferenceType(to);
}

const Type& TypeFactory::NewTupleTypeFromVector(
    const std::vector<const Type*>& members) {
  return composite_type_factory_->NewTupleType(members);
}

const Type& TypeFactory::NewUnionType(
    const std::vector<const Type*>& passed_members) {
  std::set<const Type*> members;
  for (const auto& member : passed_members) {
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
  return composite_type_factory_->NewUnionType(
      std::vector<const Type*>(members.begin(), members.end()));
}

}  // namespace ir
}  // namespace joana

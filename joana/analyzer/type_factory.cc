// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type_factory.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/types.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// TypeFactory
//
TypeFactory::TypeFactory(Zone* zone)
    : zone_(*zone),
      any_type_(*new (zone) AnyType(NextTypeId())),
      invalid_type_(*new (zone) InvalidType(NextTypeId())),
      unknown_type_(*new (zone) UnknownType(NextTypeId())),
      void_type_(*new (zone) VoidType(NextTypeId())) {
  current_type_id_ = 100;
  InstallPrimitiveTypes();
  current_type_id_ = 1000;
}

TypeFactory::~TypeFactory() = default;

const Type& TypeFactory::GetPrimitiveType(ast::TokenKind id) {
  const auto& it = primitive_type_map_.find(id);
  DCHECK(it != primitive_type_map_.end())
      << BuiltInWorld::GetInstance()->NameOf(id)
      << " should be primitive type.";
  return *it->second;
}

void TypeFactory::InstallPrimitiveTypes() {
  for (const auto id : BuiltInWorld::GetInstance()->primitive_types()) {
    const auto& name = BuiltInWorld::GetInstance()->NameOf(id);
    const auto& type = *new (&zone_) PrimitiveType(NextTypeId(), name);
    const auto& result =
        primitive_type_map_.emplace(ast::Name::KindOf(name), &type);
    DCHECK(result.second) << name << " is realdy installed.";
  }
}

int TypeFactory::NextTypeId() {
  return ++current_type_id_;
}

const Type& TypeFactory::NewClassType(Class* class_value) {
  return *new (&zone_) ClassType(NextTypeId(), class_value);
}

const Type& TypeFactory::NewTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return *new (&zone_) TypeName(NextTypeId(), name);
}

const Type& TypeFactory::NewTypeParameter(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return *new (&zone_) TypeParameter(NextTypeId(), name);
}

}  // namespace analyzer
}  // namespace joana

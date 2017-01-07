// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// PrimitiveType
//
PrimitiveType::PrimitiveType(int id, const ast::Node& node) : Type(id, node) {}
PrimitiveType::~PrimitiveType() = default;

//
// TypeName
//
TypeName::TypeName(int id, const ast::Node& node) : Type(id, node) {}
TypeName::~TypeName() = default;

//
// TypeReference
//
TypeReference::TypeReference(int id, Variable* variable)
    : Type(id, variable->node()), variable_(*variable) {}
TypeReference::~TypeReference() = default;

//
// Type
//
Type::Type(int id, const ast::Node& node) : Value(id, node) {}
Type::~Type() = default;

}  // namespace analyzer
}  // namespace joana

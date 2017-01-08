// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// GenericType
//
GenericType::GenericType(Zone* zone,
                         int id,
                         const ast::Node& node,
                         const std::vector<const TypeParameter*>& parameters)
    : Type(id, node), parameters_(zone, parameters) {}

GenericType::~GenericType() = default;

//
// PrimitiveType
//
PrimitiveType::PrimitiveType(int id, const ast::Node& node) : Type(id, node) {}
PrimitiveType::~PrimitiveType() = default;

//
// TypeApplication
//
TypeApplication::TypeApplication(Zone* zone,
                                 int id,
                                 const ast::Node& node,
                                 const GenericType& generic_type,
                                 const std::vector<Argument>& arguments)
    : Type(id, node),
      arguments_(zone, arguments),
      generic_type_(generic_type) {}

TypeApplication::~TypeApplication() = default;

//
// TypeName
//
TypeName::TypeName(int id, const ast::Node& node) : Type(id, node) {}
TypeName::~TypeName() = default;

//
// TypeParameter
//
TypeParameter::TypeParameter(int id, const ast::Node& node) : Type(id, node) {}
TypeParameter::~TypeParameter() = default;

//
// TypeReference
//
TypeReference::TypeReference(int id, Variable* variable)
    : Type(id, variable->node()), variable_(*variable) {}
TypeReference::~TypeReference() = default;

}  // namespace analyzer
}  // namespace joana

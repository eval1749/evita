// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/values.h"

#include "joana/ast/bindings.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Function
//
Function::Function(int id, const ast::Node& node, Properties* properties)
    : Object(id, node, properties) {}

Function::~Function() = default;

//
// Object
//
Object::Object(int id, const ast::Node& node, Properties* properties)
    : Value(id, node), properties_(*properties) {}

Object::~Object() = default;

//
// OrdinaryObject
//
OrdinaryObject::OrdinaryObject(int id,
                               const ast::Node& node,
                               Properties* properties)
    : Object(id, node, properties) {}

OrdinaryObject::~OrdinaryObject() = default;

//
// Property
//
Property::Property(Zone* zone,
                   int id,
                   const ast::Node& key,
                   Properties* properties)
    : ValueHolder(zone, id, key, properties) {
  DCHECK_EQ(key, ast::SyntaxCode::Name);
}

Property::~Property() = default;

//
// Undefined
//
Undefined::Undefined(int id, const ast::Node& node) : Value(id, node) {}
Undefined::~Undefined() = default;

//
// ValueHolder
//
ValueHolder::ValueHolder(Zone* zone,
                         int id,
                         const ast::Node& node,
                         Properties* properties)
    : Object(id, node, properties), assignments_(zone), references_(zone) {}

ValueHolder::~ValueHolder() = default;

//
// Variable
//
Variable::Variable(Zone* zone,
                   int id,
                   const ast::Node& name,
                   Properties* properties)
    : ValueHolder(zone, id, name, properties) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
}

Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

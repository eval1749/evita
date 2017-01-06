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
// Class
//
Class::Class(Zone* zone,
             int id,
             const ast::Node& node,
             Properties* properties,
             Object* prototype)
    : LexicalBinding(zone, id, node, properties), prototype_(*prototype) {
  DCHECK(properties);
  DCHECK(prototype);
}

Class::~Class() = default;

//
// Function
//
Function::Function(Zone* zone,
                   int id,
                   const ast::Node& node,
                   Properties* properties)
    : LexicalBinding(zone, id, node, properties) {}

Function::~Function() = default;

//
// LexicalBinding
//
LexicalBinding::LexicalBinding(Zone* zone,
                               int id,
                               const ast::Node& node,
                               Properties* properties)
    : Object(id, node, properties), assignments_(zone), references_(zone) {
  assignments_.push_back(&node);
}

LexicalBinding::~LexicalBinding() = default;

//
// Method
//
Method::Method(Zone* zone,
               int id,
               const ast::Node& node,
               Class* owner,
               Properties* properties)
    : LexicalBinding(zone, id, node, properties), owner_(*owner) {
  DCHECK(owner);
}

Method::~Method() = default;

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
    : Object(id, key, properties),
      assignments_(zone),
      key_(key),
      references_(zone) {}

Property::~Property() = default;

void Property::AddAssignment(const ast::Node& assignment) {
  assignments_.push_back(&assignment);
}

void Property::AddReference(const ast::Node& reference) {
  references_.push_back(&reference);
}

//
// Undefined
//
Undefined::Undefined(int id, const ast::Node& node) : Value(id, node) {}
Undefined::~Undefined() = default;

//
// Variable
//
Variable::Variable(Zone* zone,
                   int id,
                   const ast::Node& assignment,
                   const ast::Node& name,
                   Properties* properties)
    : LexicalBinding(zone, id, name, properties), assignment_(assignment) {
  DCHECK(name == ast::SyntaxCode::Name ||
         name == ast::SyntaxCode::BindingNameElement)
      << name;
}

Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

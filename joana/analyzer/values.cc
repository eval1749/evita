// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/values.h"

#include "joana/ast/bindings.h"
#include "joana/ast/expressions.h"
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
             ClassKind kind,
             const std::vector<const TypeParameter*>& parameters,
             Properties* properties)
    : Object(id, node, properties),
      base_classes_(zone),
      kind_(kind),
      parameters_(zone, parameters) {}

Class::~Class() = default;

//
// ConstructedClass
//
ConstructedClass::ConstructedClass(int id,
                                   Class* generic_class,
                                   const std::vector<const Type*>& arguments)
    : Value(id, generic_class->node()),
      generic_class_(*generic_class),
      number_of_arguments_(arguments.size()) {
  DCHECK(generic_class);
  DCHECK_EQ(arguments.size(), generic_class->parameters().size());
  DCHECK_GE(arguments.size(), static_cast<size_t>(1));
  auto* runner = arguments_;
  for (const auto& argument : arguments) {
    *runner = argument;
    ++runner;
  }
}

ConstructedClass::~ConstructedClass() = default;

BlockRange<const Type*> ConstructedClass::arguments() const {
  return BlockRange<const Type*>(arguments_, number_of_arguments_);
}

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
                   Visibility visibility,
                   const ast::Node& key,
                   Properties* properties)
    : ValueHolder(zone, id, key, properties), visibility_(visibility) {
  DCHECK(key == ast::SyntaxCode::Name || ast::IsKnownSymbol(key)) << key;
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
                   VariableKind kind,
                   const ast::Node& name,
                   Properties* properties)
    : ValueHolder(zone, id, name, properties), kind_(kind) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
}

Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

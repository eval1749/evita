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
Function::Function(Zone* zone, int id, const ast::Node& node)
    : LexicalBinding(zone, id, node) {}

Function::~Function() = default;

//
// LexicalBinding
//
LexicalBinding::LexicalBinding(Zone* zone, int id, const ast::Node& node)
    : Value(id, node), assignments_(zone), references_(zone) {
  assignments_.push_back(&node);
}

LexicalBinding::~LexicalBinding() = default;

//
// Property
//
Property::Property(Zone* zone, int id, const ast::Node& node)
    : LexicalBinding(zone, id, node) {}

Property::~Property() = default;

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
                   const ast::Node& name)
    : LexicalBinding(zone, id, name), assignment_(assignment) {
  DCHECK(name == ast::SyntaxCode::Name ||
         name == ast::SyntaxCode::BindingNameElement)
      << name;
}

Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// Function
//
Function::Function(Zone* zone, const ast::Node& node)
    : LexicalBinding(zone, node) {}
Function::~Function() = default;

//
// LexicalBinding
//
LexicalBinding::LexicalBinding(Zone* zone, const ast::Node& node)
    : Value(node), assignments_(zone), references_(zone) {
  assignments_.push_back(&node);
}

LexicalBinding::~LexicalBinding() = default;

//
// Property
//
Property::Property(Zone* zone, const ast::Node& node)
    : LexicalBinding(zone, node) {}
Property::~Property() = default;

//
// Variable
//
Variable::Variable(Zone* zone, const ast::Node& node)
    : LexicalBinding(zone, node) {}
Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

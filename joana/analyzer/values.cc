// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// Function
//
Function::Function(const ast::Node& node) : Value(node) {}
Function::~Function() = default;

//
// Property
//
Property::Property(const ast::Node& node) : Value(node) {}
Property::~Property() = default;

//
// Variable
//
Variable::Variable(const ast::Node& node) : Value(node) {}
Variable::~Variable() = default;

}  // namespace analyzer
}  // namespace joana

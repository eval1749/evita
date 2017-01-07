// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/types.h"

namespace joana {
namespace analyzer {

//
// PrimitiveType
//
PrimitiveType::PrimitiveType(int id, const ast::Node& node) : Type(id, node) {}
PrimitiveType::~PrimitiveType() = default;

//
// Type
//
Type::Type(int id, const ast::Node& node) : Value(id, node) {}
Type::~Type() = default;

}  // namespace analyzer
}  // namespace joana

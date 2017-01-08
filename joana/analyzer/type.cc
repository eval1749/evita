// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type.h"

namespace joana {
namespace analyzer {

//
// Type
//
Type::Type(int id, const ast::Node& node) : id_(id), node_(node) {}
Type::~Type() = default;

}  // namespace analyzer
}  // namespace joana

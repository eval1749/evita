// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ir/primitive_types.h"

namespace joana {
namespace ir {

//
// Primitve Types
//
#define V(capital, underscoe)               \
  capital##Type::capital##Type() = default; \
  capital##Type::~capital##Type() = default;

FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V

//
// PrimitiveType
//
PrimitiveType::PrimitiveType() = default;
PrimitiveType::~PrimitiveType() = default;

}  // namespace ir
}  // namespace joana

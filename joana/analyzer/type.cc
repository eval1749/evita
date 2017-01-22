// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type.h"

namespace joana {
namespace analyzer {

//
// Type
//
Type::Type(int id) : id_(id) {}
Type::~Type() = default;

bool Type::operator==(const Type& other) const {
  return this == &other;
}

bool Type::operator==(const Type* other) const {
  return this == other;
}

bool Type::operator!=(const Type& other) const {
  return this != &other;
}

bool Type::operator!=(const Type* other) const {
  return this != other;
}

}  // namespace analyzer
}  // namespace joana

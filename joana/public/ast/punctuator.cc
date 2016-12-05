// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/punctuator.h"

namespace joana {
namespace ast {

Punctuator::Punctuator(const SourceCodeRange& location, Kind kind)
    : Node(location), kind_(kind) {}

Punctuator::~Punctuator() = default;

}  // namespace ast
}  // namespace joana

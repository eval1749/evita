// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/public/ast/name.h"

namespace joana {
namespace ast {

Name::Name(const SourceCodeRange& range, int number)
    : Node(range), number_(number) {}

Name::~Name() = default;

// Implements |Node| members
void Name::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", " << number_;
}

}  // namespace ast
}  // namespace joana

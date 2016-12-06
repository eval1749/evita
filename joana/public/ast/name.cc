// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/name.h"

namespace joana {
namespace ast {

Name::Name(const SourceCodeRange& range) : Node(range) {}

Name::~Name() = default;

}  // namespace ast
}  // namespace joana

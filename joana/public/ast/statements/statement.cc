// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/statements/statement.h"

namespace joana {
namespace ast {

Statement::Statement(const SourceCodeRange& location)
    : ContainerNode(location) {}

Statement::~Statement() = default;

}  // namespace ast
}  // namespace joana

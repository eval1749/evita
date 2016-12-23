// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/module.h"

namespace joana {
namespace ast {

Module::Module(const SourceCodeRange& range, const StatementList& statements)
    : NodeTemplate(&statements, range) {}

Module::~Module() = default;

}  // namespace ast
}  // namespace joana

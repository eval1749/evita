// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_COMPILATION_UNITS_H_
#define JOANA_AST_COMPILATION_UNITS_H_

#include <unordered_map>

#include "joana/ast/syntax.h"
#include "joana/base/memory/zone_unordered_map.h"

namespace joana {
namespace ast {

DECLARE_AST_SYNTAX_0(Externs)
DECLARE_AST_SYNTAX_0(Module)
DECLARE_AST_SYNTAX_0(Script)

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_COMPILATION_UNITS_H_

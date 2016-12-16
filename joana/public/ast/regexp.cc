// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/regexp.h"

namespace joana {
namespace ast {

//
// RegExp
//
RegExp::RegExp(const SourceCodeRange& range) : Node(range) {}

RegExp::~RegExp() = default;

}  // namespace ast
}  // namespace joana

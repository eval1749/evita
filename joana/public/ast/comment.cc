// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/comment.h"

namespace joana {
namespace ast {

Comment::Comment(const SourceCodeRange& location) : Node(location) {}

Comment::~Comment() = default;

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/template.h"

namespace joana {
namespace ast {

Template::Template(const SourceCodeRange& range) : ContainerNode(range) {}

Template::~Template() = default;

}  // namespace ast
}  // namespace joana

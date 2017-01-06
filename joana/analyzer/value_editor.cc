// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/value_editor.h"

#include "joana/analyzer/values.h"

namespace joana {
namespace analyzer {

//
// Value::Editor
//
Value::Editor::Editor() = default;
Value::Editor::~Editor() = default;

void Value::Editor::AddAssignment(LexicalBinding* binding,
                                  const ast::Node& node) {
  binding->assignments_.push_back(&node);
}

}  // namespace analyzer
}  // namespace joana

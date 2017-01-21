// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/value_editor.h"

#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"

namespace joana {
namespace analyzer {

namespace {

bool IsValidAssignment(const ast::Node& node) {
  return node.Is<ast::AssignmentExpression>() ||
         node.Is<ast::BindingNameElement>() || node.Is<ast::Class>() ||
         node.Is<ast::JsDocDocument>() || node.Is<ast::Function>() ||
         node.Is<ast::Method>() ||
         node == BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Global);
}

}  // namespace

//
// Value::Editor
//
Value::Editor::Editor() = default;
Value::Editor::~Editor() = default;

void Value::Editor::AddAssignment(ValueHolder* binding, const ast::Node& node) {
  DCHECK(IsValidAssignment(node)) << *binding << ' ' << node;
  binding->assignments_.push_back(&node);
}

void Value::Editor::AddMethod(Class* class_value, Function* method) {
  class_value->methods_.push_back(method);
}

}  // namespace analyzer
}  // namespace joana

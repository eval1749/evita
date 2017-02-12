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
         node.Is<ast::ComputedMemberExpression>() ||
         node.Is<ast::JsDocDocument>() || node.Is<ast::Function>() ||
         node.Is<ast::Method>() || node.Is<ast::MemberExpression>() ||
         node == BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Global) ||
         node == BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Array) ||
         node == BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Object);
}

}  // namespace

//
// Value::Editor
//
Value::Editor::Editor() = default;
Value::Editor::~Editor() = default;

void Value::Editor::AddAssignment(const ValueHolder& binding,
                                  const ast::Node& node) {
  DCHECK(IsValidAssignment(node)) << binding << ' ' << node;
  const_cast<ValueHolder&>(binding).data_.assignments_.push_back(&node);
}

void Value::Editor::SetClassHeritage(const Class& class_value,
                                     const std::vector<const Class*>& classes) {
  auto& base_classes = const_cast<Class&>(class_value).base_classes_;
  DCHECK(base_classes.empty()) << class_value;
  base_classes.reserve(classes.size());
  base_classes.insert(base_classes.begin(), classes.begin(), classes.end());
}

void Value::Editor::SetClassList(const Class& class_value,
                                 const std::vector<const Class*>& classes) {
  auto& class_list = const_cast<Class&>(class_value).class_list_;
  DCHECK(class_list.empty()) << class_value;
  class_list.reserve(classes.size());
  class_list.insert(class_list.begin(), classes.begin(), classes.end());
}

}  // namespace analyzer
}  // namespace joana

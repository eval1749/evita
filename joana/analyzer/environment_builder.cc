// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/environment_builder.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/value.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/value_forward.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/node_forward.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Name& NameOf(const ast::Node& node) {
  if (auto* name = node.TryAs<ast::Name>())
    return *name;
  if (auto* element = node.TryAs<ast::BindingNameElement>())
    return element->name();
  NOTREACHED() << node << " is not name node.";
  return node.As<ast::Name>();
}

}  // namespace

//
// EnvironmentBuilder
//
EnvironmentBuilder::EnvironmentBuilder(Context* context)
    : Pass(context), environment_(&factory().global_environment()) {}

EnvironmentBuilder::~EnvironmentBuilder() = default;

void EnvironmentBuilder::Load(const ast::Node& node) {
  const auto& module = node.As<ast::Module>();
  for (const auto& statement : module.statements())
    ProcessStatement(statement);
}

// Binding helpers
void EnvironmentBuilder::BindToFunction(const ast::Name& name,
                                        const ast::Declaration& declaration) {
  if (auto* present = environment_->BindingOf(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): We should bind to |Constructor| if |declaration| has
  // "@constructor" annotation.
  environment_->Bind(name, &factory().NewFunction(declaration));
}

void EnvironmentBuilder::BindToVariable(const ast::Node& assignment,
                                        const ast::Node& name_node) {
  const auto& name = NameOf(name_node);
  if (auto* present = environment_->BindingOf(name)) {
    if (present->node().Is<ast::VarStatement>()) {
      // TODO(eval1749): We should report error if |present| has type
      // annotation.
      Value::Editor().AddAsignment(&present->As<LexicalBinding>(), name_node);
      return;
    }
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): Expose global "var" binding to global object.
  environment_->Bind(name, &factory().NewVariable(assignment, name_node));
}

// AST node handlers
void EnvironmentBuilder::ProcessBindingElement(
    const ast::Node& declaration,
    const ast::BindingElement& element) {
  if (auto* array = element.TryAs<ast::ArrayBindingPattern>()) {
    for (const auto& element : array->elements())
      ProcessBindingElement(declaration, element);
    return;
  }
  if (element.Is<ast::BindingCommaElement>())
    return;
  if (element.Is<ast::BindingInvalidElement>())
    return;
  if (auto* named = element.TryAs<ast::BindingNameElement>())
    return BindToVariable(declaration, element);
  if (auto* property = element.TryAs<ast::BindingProperty>())
    return ProcessBindingElement(declaration, property->element());
  if (auto* rest = element.TryAs<ast::BindingRestElement>())
    return ProcessBindingElement(declaration, rest->element());
  if (auto* array = element.TryAs<ast::ObjectBindingPattern>()) {
    for (const auto& element : array->elements())
      ProcessBindingElement(declaration, element);
    return;
  }
  NOTREACHED() << "Unsupported " << element;
}

void EnvironmentBuilder::ProcessDeclaration(
    const ast::Declaration& declaration) {
  if (auto* klass = declaration.TryAs<ast::Class>()) {
    // TODO(eval1749): Report warning for anonymous class
    return BindToFunction(klass->name().As<ast::Name>(), declaration);
  }
  if (auto* fun = declaration.TryAs<ast::Function>()) {
    // TODO(eval1749): Report warning for anonymous class
    return BindToFunction(fun->name().As<ast::Name>(), declaration);
  }
  NOTREACHED() << "Unsupported " << declaration;
}

void EnvironmentBuilder::ProcessStatement(const ast::Statement& statement) {
  if (auto* decl = statement.TryAs<ast::DeclarationStatement>())
    return ProcessDeclaration(decl->declaration());
  if (auto* const_stmt = statement.TryAs<ast::ConstStatement>())
    return ProcessVariables(*const_stmt);
  if (auto* let_stmt = statement.TryAs<ast::LetStatement>())
    return ProcessVariables(*let_stmt);
  if (auto* var_stmt = statement.TryAs<ast::VarStatement>())
    return ProcessVariables(*var_stmt);
}

void EnvironmentBuilder::ProcessVariables(
    const ast::VariableDeclaration& statement) {
  for (const auto& element : statement.elements())
    ProcessBindingElement(statement, element);
}

}  // namespace analyzer
}  // namespace joana

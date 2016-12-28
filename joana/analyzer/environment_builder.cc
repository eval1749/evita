// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/environment_builder.h"

#include "joana/analyzer/environment.h"
#include "joana/analyzer/factory.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/module.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// EnvironmentBuilder
//
EnvironmentBuilder::EnvironmentBuilder(Factory* factory)
    : environment_(factory->global_environment()), factory_(*factory) {}

EnvironmentBuilder::~EnvironmentBuilder() = default;

void EnvironmentBuilder::Load(const ast::Node& node) {
  const auto& module = node.As<ast::Module>();
  for (const auto& statement : module.statements())
    ProcessStatement(statement);
}

// Binding helpers
void EnvironmentBuilder::BindToFunction(const ast::Name& name,
                                        const ast::Declaration& declaration) {
  if (auto* binding = environment_->BindingOf(name)) {
    // AddError(name.range(), ErrorCode::ENVIRONMENT_ALREADY_BOUND);
    return;
  }
  environment_->Bind(name, factory_.NewFunction(declaration));
}

// AST node handlers
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
}

void EnvironmentBuilder::ProcessVariable(const ast::Statement& statement,
                                         const ast::Expression& expression) {}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

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
// EnvironmentBuilder::LocalEnvironment
//
class EnvironmentBuilder::LocalEnvironment final {
 public:
  LocalEnvironment(EnvironmentBuilder* builder, const ast::Node& owner);
  ~LocalEnvironment();

  const LocalEnvironment* outer() const { return outer_; }
  LocalEnvironment* outer() { return outer_; }

  void Bind(const ast::Name& name, Value* value);
  Value* Find(const ast::Name& name) const;

 private:
  EnvironmentBuilder& builder_;
  LocalEnvironment* const outer_;
  const ast::Node& owner_;
  std::unordered_map<int, Value*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(LocalEnvironment);
};

EnvironmentBuilder::LocalEnvironment::LocalEnvironment(
    EnvironmentBuilder* builder,
    const ast::Node& owner)
    : builder_(*builder), outer_(builder_.environment_), owner_(owner) {
  builder_.environment_ = this;
}

EnvironmentBuilder::LocalEnvironment::~LocalEnvironment() {
  builder_.environment_ = outer_;
}

void EnvironmentBuilder::LocalEnvironment::Bind(const ast::Name& name,
                                                Value* value) {
  const auto& result = value_map_.emplace(name.number(), value);
  DCHECK(result.second);
}

Value* EnvironmentBuilder::LocalEnvironment::Find(const ast::Name& name) const {
  const auto& it = value_map_.find(name.number());
  return it == value_map_.end() ? nullptr : it->second;
}

//
// EnvironmentBuilder
//
EnvironmentBuilder::EnvironmentBuilder(Context* context) : Pass(context) {}

EnvironmentBuilder::~EnvironmentBuilder() = default;

// The entry point. |node| is one of |ast::Externs|, |ast::Module| or
// |ast::Script|.
void EnvironmentBuilder::Load(const ast::Node& node) {
  auto& global_environment = factory().global_environment();
  toplevel_environment_ =
      node.Is<ast::Module>()
          ? &factory().NewEnvironment(&global_environment, node)
          : &global_environment;
  Visit(node);
}

// Binding helpers
void EnvironmentBuilder::BindToFunction(const ast::Name& name,
                                        const ast::Declaration& declaration) {
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      return;
    }
    environment_->Bind(name, &factory().NewFunction(declaration));
  }

  if (auto* present = toplevel_environment_->BindingOf(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): We should bind to |Constructor| if |declaration| has
  // "@constructor" annotation.
  toplevel_environment_->Bind(name, &factory().NewFunction(declaration));
}

void EnvironmentBuilder::BindToVariable(const ast::Node& origin,
                                        const ast::Node& name_node) {
  const auto& name = NameOf(name_node);
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      if (present->node().Is<ast::VarStatement>() &&
          origin.Is<ast::VarStatement>()) {
        // TODO(eval1749): We should report error if |present| has type
        // annotation.
        Value::Editor().AddAsignment(&present->As<LexicalBinding>(), name_node);
        return;
      }
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      return;
    }
    environment_->Bind(name, &factory().NewVariable(origin, name_node));
    return;
  }
  if (auto* present = toplevel_environment_->BindingOf(name)) {
    if (present->node().Is<ast::VarStatement>() &&
        origin.Is<ast::VarStatement>()) {
      // TODO(eval1749): We should report error if |present| has type
      // annotation.
      Value::Editor().AddAsignment(&present->As<LexicalBinding>(), name_node);
      return;
    }
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): Expose global "var" binding to global object.
  toplevel_environment_->Bind(name, &factory().NewVariable(origin, name_node));
}

// AST node handlers
void EnvironmentBuilder::ProcessCompilationUnit(
    const ast::CompilationUnit& node) {
  for (const auto& statement : node.statements())
    Visit(statement);
}

void EnvironmentBuilder::ProcessVariables(
    const ast::VariableDeclaration& statement) {
  variable_origin_ = &statement;
  for (const auto& element : statement.elements())
    Visit(element);
}

//
// ast::NodeVisitor members
//

// Binding elements
void EnvironmentBuilder::VisitArrayBindingPattern(
    const ast::ArrayBindingPattern& node) {
  for (const auto& element : node.elements())
    Visit(element);
}

void EnvironmentBuilder::VisitBindingCommaElement(
    const ast::BindingCommaElement& node) {}

void EnvironmentBuilder::VisitBindingInvalidElement(
    const ast::BindingInvalidElement& node) {}

void EnvironmentBuilder::VisitBindingNameElement(
    const ast::BindingNameElement& node) {
  BindToVariable(*variable_origin_, node);
}

void EnvironmentBuilder::VisitBindingProperty(
    const ast::BindingProperty& node) {
  Visit(node.element());
}

void EnvironmentBuilder::VisitBindingRestElement(
    const ast::BindingRestElement& node) {
  Visit(node.element());
}

void EnvironmentBuilder::VisitObjectBindingPattern(
    const ast::ObjectBindingPattern& node) {
  for (const auto& element : node.elements())
    Visit(element);
}

// Compilation units
void EnvironmentBuilder::VisitExterns(const ast::Externs& node) {
  ProcessCompilationUnit(node);
}

void EnvironmentBuilder::VisitModule(const ast::Module& node) {
  ProcessCompilationUnit(node);
}

void EnvironmentBuilder::VisitScript(const ast::Script& node) {
  ProcessCompilationUnit(node);
}

// Declarations
void EnvironmentBuilder::VisitClass(const ast::Class& node) {
  // TODO(eval1749): Report warning for anonymous class
  BindToFunction(node.name().As<ast::Name>(), node);
}

void EnvironmentBuilder::VisitFunction(const ast::Function& node) {
  // TODO(eval1749): Report warning for anonymous class
  BindToFunction(node.name().As<ast::Name>(), node);
  LocalEnvironment environment(this, node);
  variable_origin_ = &node;
  Visit(node.parameter_list());
}

// Expressions
void EnvironmentBuilder::VisitBinaryExpression(
    const ast::BinaryExpression& node) {
  Visit(node.lhs());
  Visit(node.rhs());
}

void EnvironmentBuilder::VisitParameterList(const ast::ParameterList& node) {
  for (const auto& parameter : node)
    Visit(parameter);
}

void EnvironmentBuilder::VisitReferenceExpression(
    const ast::ReferenceExpression& node) {
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = environment_->Find(node.name())) {
      factory().RegisterValue(node, present);
      return;
    }
  }
  auto* present = toplevel_environment_->BindingOf(node.name());
  if (!present)
    return;
  factory().RegisterValue(node, present);
}

// Statements
void EnvironmentBuilder::VisitBlockStatement(const ast::BlockStatement& node) {
  LocalEnvironment environment(this, node);
  for (const auto& statement : node.statements())
    Visit(statement);
}

void EnvironmentBuilder::VisitDeclarationStatement(
    const ast::DeclarationStatement& node) {
  Visit(node.declaration());
}

void EnvironmentBuilder::VisitExpressionStatement(
    const ast::ExpressionStatement& node) {
  Visit(node.expression());
}

void EnvironmentBuilder::VisitConstStatement(const ast::ConstStatement& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::VisitLetStatement(const ast::LetStatement& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::VisitReturnStatement(
    const ast::ReturnStatement& node) {
  Visit(node.expression());
}

void EnvironmentBuilder::VisitThrowStatement(const ast::ThrowStatement& node) {
  Visit(node.expression());
}

void EnvironmentBuilder::VisitVarStatement(const ast::VarStatement& node) {
  ProcessVariables(node);
}

}  // namespace analyzer
}  // namespace joana

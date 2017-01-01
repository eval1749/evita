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
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node& NameOf(const ast::Node& node) {
  if (node == ast::SyntaxCode::Name)
    return node;
  if (node == ast::SyntaxCode::BindingNameElement)
    return node.child_at(0);
  NOTREACHED() << node << " is not name node.";
  return node;
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

  void Bind(const ast::Node& name, Value* value);
  Value* Find(const ast::Node& name) const;

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

void EnvironmentBuilder::LocalEnvironment::Bind(const ast::Node& name,
                                                Value* value) {
  const auto& result = value_map_.emplace(name.name_id(), value);
  DCHECK(result.second);
}

Value* EnvironmentBuilder::LocalEnvironment::Find(const ast::Node& name) const {
  const auto& it = value_map_.find(name.name_id());
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
void EnvironmentBuilder::BindToFunction(const ast::Node& name,
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
void EnvironmentBuilder::ProcessVariables(
    const ast::VariableDeclaration& statement) {
  variable_origin_ = &statement;
  for (const auto& element : ast::NodeTraversal::ChildNodesOf(statement))
    Visit(element);
}

void EnvironmentBuilder::Visit(const ast::Node& node) {
  NOTREACHED() << "Dummy";
}

//
// ast::NodeVisitor members
//

// Binding elements
void EnvironmentBuilder::VisitBindingNameElement(
    const ast::BindingNameElement& node) {
  BindToVariable(*variable_origin_, node);
}

// Declarations
void EnvironmentBuilder::VisitClass(const ast::Class& node) {
  // TODO(eval1749): Report warning for anonymous class
  BindToFunction(ast::ClassTag::NameOf(node), node);
}

void EnvironmentBuilder::VisitFunction(const ast::Function& node) {
  // TODO(eval1749): Report warning for anonymous class
  BindToFunction(ast::FunctionTag::NameOf(node), node);
  LocalEnvironment environment(this, node);
  variable_origin_ = &node;
  Visit(ast::FunctionTag::ParametersOf(node));
}

// Expressions
void EnvironmentBuilder::VisitReferenceExpression(
    const ast::ReferenceExpression& node) {
  const auto& name = ast::ReferenceExpressionTag::NameOf(node);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = environment_->Find(name)) {
      factory().RegisterValue(node, present);
      return;
    }
  }
  auto* present = toplevel_environment_->BindingOf(name);
  if (!present)
    return;
  factory().RegisterValue(node, present);
}

// Statements
void EnvironmentBuilder::VisitBlockStatement(const ast::BlockStatement& node) {
  LocalEnvironment environment(this, node);
  for (const auto& statement : ast::NodeTraversal::ChildNodesOf(node))
    Visit(statement);
}

void EnvironmentBuilder::VisitConstStatement(const ast::ConstStatement& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::VisitLetStatement(const ast::LetStatement& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::VisitVarStatement(const ast::VarStatement& node) {
  ProcessVariables(node);
}

}  // namespace analyzer
}  // namespace joana

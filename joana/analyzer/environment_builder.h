// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include <unordered_set>

#include "joana/analyzer/pass.h"

#include "base/logging.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/syntax_visitor.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Context;
class Environment;
enum class VariableKind;

//
// EnvironmentBuilder
//
class EnvironmentBuilder final : public Pass, public ast::SyntaxVisitor {
 public:
  explicit EnvironmentBuilder(Context* context);
  ~EnvironmentBuilder();

  void RunOn(const ast::Node& node);

 private:
  class LocalEnvironment;

  // Binding helpers
  void BindToClass(const ast::Node& name, const ast::Node& declaration);

  void BindToFunction(const ast::Node& name, const ast::Node& declaration);

  void BindToVariable(const ast::Node& origin, const ast::Node& name);

  // Process AST nodes
  void ProcessVariables(const ast::Node& declaration);

  // ast::NodeVisitor implementations

  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node) final;

  // Binding elements
  void Visit(const ast::BindingNameElementSyntax& syntax,
             const ast::Node& node) final;

  // Declarations
  void Visit(const ast::ClassSyntax& syntax, const ast::Node& node) final;
  void Visit(const ast::FunctionSyntax& syntax, const ast::Node& node) final;
  void Visit(const ast::MethodSyntax& syntax, const ast::Node& node) final;

  // Expressions
  void Visit(const ast::ReferenceExpressionSyntax& syntax,
             const ast::Node& node) final;

  // Statement
  void Visit(const ast::BlockStatementSyntax& syntax,
             const ast::Node& node) final;
  void Visit(const ast::ConstStatementSyntax& syntax,
             const ast::Node& node) final;
  void Visit(const ast::LetStatementSyntax& syntax,
             const ast::Node& node) final;
  void Visit(const ast::VarStatementSyntax& syntax,
             const ast::Node& node) final;

  LocalEnvironment* environment_ = nullptr;

  // The toplevel environment
  Environment* toplevel_environment_ = nullptr;

  // |ast::Function|, |ast::Method| or |ast::VariableDeclaration|.
  const ast::Node* variable_origin_ = nullptr;

#if DCHECK_IS_ON()
  std::unordered_set<const ast::Node*> visited_nodes_;
#endif

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

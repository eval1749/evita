// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include <unordered_set>

#include "joana/analyzer/pass.h"

#include "base/logging.h"

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
class EnvironmentBuilder final : public Pass {
 public:
  explicit EnvironmentBuilder(Context* context);
  ~EnvironmentBuilder();

  void Load(const ast::Node& node);

 private:
  class LocalEnvironment;

  void Visit(const ast::Node& node);
  void VisitChildNodes(const ast::Node& node);

  // Binding helpers
  void BindToFunction(const ast::Node& name, const ast::Node& declaration);

  void BindToVariable(const ast::Node& origin, const ast::Node& name);

  // Process AST nodes
  void ProcessVariables(const ast::Node& declaration);

  // ast::NodeVisitor implementations

  // Binding elements
  void VisitBindingNameElement(const ast::Node& node);

  // Declarations
  void VisitClass(const ast::Node& node);
  void VisitFunction(const ast::Node& node);

  // Expressions
  void VisitReferenceExpression(const ast::Node& node);

  // Statement
  void VisitBlockStatement(const ast::Node& node);

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

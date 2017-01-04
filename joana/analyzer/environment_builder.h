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

class Class;
class Context;
class Environment;
class Function;
class Properties;
class Value;
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
  void Bind(const ast::Node& name, Value* value);

  void BindToVariable(const ast::Node& origin, const ast::Node& name);

  void ProcessAssignmentExpressionWithAnnotation(const ast::Node& node,
                                                 const ast::Node& annotation);

  void ProcessMemberExpressionWithAnnotation(const ast::Node& node,
                                             const ast::Node& annotation);

  // Process AST nodes
  void ProcessVariables(const ast::Node& declaration);

  // ast::NodeVisitor implementations

  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node) final;

  // Binding elements
  void Visit(const ast::BindingNameElement& syntax,
             const ast::Node& node) final;

  // Declarations
  void Visit(const ast::Annotation& syntax, const ast::Node& node) final;
  void Visit(const ast::Class& syntax, const ast::Node& node) final;
  void Visit(const ast::Function& syntax, const ast::Node& node) final;
  void Visit(const ast::Method& syntax, const ast::Node& node) final;

  // Expressions
  void Visit(const ast::MemberExpression& syntax, const ast::Node& node) final;
  void Visit(const ast::ReferenceExpression& syntax,
             const ast::Node& node) final;

  // Statement
  void Visit(const ast::BlockStatement& syntax, const ast::Node& node) final;
  void Visit(const ast::ExpressionStatement& syntax,
             const ast::Node& node) final;
  void Visit(const ast::ConstStatement& syntax, const ast::Node& node) final;
  void Visit(const ast::LetStatement& syntax, const ast::Node& node) final;
  void Visit(const ast::VarStatement& syntax, const ast::Node& node) final;

  const ast::Node* annotation_ = nullptr;

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

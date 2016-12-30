// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include "joana/analyzer/pass.h"

#include "joana/ast/empty_node_visitor.h"

namespace joana {

namespace ast {
class BindingElement;
class CompilationUnit;
class Declaration;
class Expression;
class Name;
class Node;
class Statement;
class VariableDeclaration;
}

namespace analyzer {

class Context;
class Environment;
enum class VariableKind;

//
// EnvironmentBuilder
//
class EnvironmentBuilder final : public Pass, public ast::EmptyNodeVisitor {
 public:
  explicit EnvironmentBuilder(Context* context);
  ~EnvironmentBuilder();

  void Load(const ast::Node& node);

 private:
  class LocalEnvironment;

  // Binding helpers
  void BindToFunction(const ast::Name& name,
                      const ast::Declaration& declaration);

  void BindToVariable(const ast::Node& origin, const ast::Node& name);

  // Process AST nodes
  void ProcessCompilationUnit(const ast::CompilationUnit& node);
  void ProcessVariables(const ast::VariableDeclaration& declaration);

// ast::NodeVisitor implementations

// Binding elements
#define V(name) void Visit##name(const ast::name& node) final;
  FOR_EACH_AST_BINDING_ELEMENT(V)
#undef V

  // Compilation units
  void VisitExterns(const ast::Externs& node) final;
  void VisitModule(const ast::Module& node) final;
  void VisitScript(const ast::Script& node) final;

  // Declarations
  void VisitClass(const ast::Class& node) final;
  void VisitFunction(const ast::Function& node) final;

  // Expressions
  void VisitBinaryExpression(const ast::BinaryExpression& node) final;
  void VisitParameterList(const ast::ParameterList& node) final;
  void VisitReferenceExpression(const ast::ReferenceExpression& node) final;

  // Statement
  void VisitBlockStatement(const ast::BlockStatement& node) final;
  void VisitDeclarationStatement(const ast::DeclarationStatement& node) final;
  void VisitConstStatement(const ast::ConstStatement& node) final;
  void VisitExpressionStatement(const ast::ExpressionStatement& node) final;
  void VisitLetStatement(const ast::LetStatement& node) final;
  void VisitReturnStatement(const ast::ReturnStatement& node) final;
  void VisitThrowStatement(const ast::ThrowStatement& node) final;
  void VisitVarStatement(const ast::VarStatement& node) final;

  LocalEnvironment* environment_ = nullptr;

  // The toplevel environment
  Environment* toplevel_environment_ = nullptr;

  // |ast::Function|, |ast::Method| or |ast::VariableDeclaration|.
  const ast::Node* variable_origin_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

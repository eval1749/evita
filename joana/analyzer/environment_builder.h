// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_
#define JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

#include <unordered_set>
#include <vector>

#include "joana/analyzer/pass.h"

#include "joana/ast/syntax_forward.h"
#include "joana/ast/syntax_visitor.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Context;
class Environment;
class Function;
class GenericType;
class Properties;
class Type;
class TypeParameter;
class Value;
class Variable;
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

  Variable& BindToVariable(const ast::Node& name);

  // Bind |name| to |type| in current environment.
  void BindType(const ast::Node& name, const Type& type);

  // Bind type parameters of |type| in current environment.
  void BindTypeParameters(const GenericType& type);

  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

  // Assign |Class| value to |node|.
  void ProcessClass(const ast::Node& node, const ast::Node* maybe_document);

  // Returns @constructor, @interface or @record tag.
  const ast::Node* ProcessClassTag(const ast::Node& document);

  // Assign |Function| value to |node|.
  void ProcessFunction(const ast::Node& node, const ast::Node* maybe_document);

  std::vector<const TypeParameter*> ProcessTemplateTag(
      const ast::Node& document);

  Variable& ResolveVariableName(const ast::Node& name);

  void VisitChildNodes(const ast::Node& node);

  // ast::NodeVisitor implementations

  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node) final;

  // Binding elements
  void VisitInternal(const ast::BindingNameElement& syntax,
                     const ast::Node& node) final;

  // Declarations
  void VisitInternal(const ast::Annotation& syntax,
                     const ast::Node& node) final;
  void VisitInternal(const ast::Class& syntax, const ast::Node& node) final;
  void VisitInternal(const ast::Function& syntax, const ast::Node& node) final;
  void VisitInternal(const ast::Method& syntax, const ast::Node& node) final;

  // Expressions
  void VisitInternal(const ast::AssignmentExpression& syntax,
                     const ast::Node& node) final;
  void VisitInternal(const ast::ComputedMemberExpression& syntax,
                     const ast::Node& node) final;
  void VisitInternal(const ast::MemberExpression& syntax,
                     const ast::Node& node) final;
  void VisitInternal(const ast::ReferenceExpression& syntax,
                     const ast::Node& node) final;

  // Statement
  void VisitInternal(const ast::BlockStatement& syntax,
                     const ast::Node& node) final;

  // Types
  void VisitInternal(const ast::TypeName& syntax, const ast::Node& node) final;

  std::vector<const ast::Node*> ancestors_;

  LocalEnvironment* environment_ = nullptr;

  // The toplevel environment
  Environment* toplevel_environment_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(EnvironmentBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ENVIRONMENT_BUILDER_H_

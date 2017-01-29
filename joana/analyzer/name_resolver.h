// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_NAME_RESOLVER_H_
#define JOANA_ANALYZER_NAME_RESOLVER_H_

#include <memory>
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

class Annotation;
class Class;
enum class ClassKind;
class Context;
class Function;
class Properties;
class Property;
class Type;
class TypeParameter;
class Value;
class Variable;
enum class VariableKind;

//
// NameResolver
//
class NameResolver final : public Pass, public ast::SyntaxVisitor {
 public:
  explicit NameResolver(Context* context);
  ~NameResolver() final;

  void RunOn(const ast::Node& node) final;

 private:
  class Environment;

  // Bind |name| to |type| in current environment.
  void BindType(const ast::Node& name, const Type& type);

  // Bind |name| as |variable| in current environment.
  Variable& BindVariable(VariableKind kind, const ast::Node& name);

  // Bind type parameters of |type| in current environment.
  void BindTypeParameters(const Class& class_value);

  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

  Property& GetOrNewProperty(Properties* properties, const ast::Node& node);

  Class& NewClass(ClassKind kind,
                  const ast::Node& name,
                  const ast::Node& node,
                  const std::vector<const TypeParameter*>& parameters,
                  Properties* properties = nullptr);

  void ProcessAssignment(const ast::Node& lhs,
                         const ast::Node* maybe_rhs,
                         const ast::Node& name,
                         const Annotation& annotation);

  // Assign |Class| value to |node|.
  void ProcessClass(const ast::Node& node,
                    const Annotation& annotation,
                    const ast::Node* alias);

  // BInd name of @param tags.
  void ProcessParamTags(const Annotation& annotation);

  // Assign |Function| value to |node|.
  void ProcessFunction(const ast::Node& node,
                       const Annotation& annotation,
                       const ast::Node* alias);

  void ProcessPropertyAssignment(const ast::Node& lhs,
                                 const ast::Node* maybe_rhs,
                                 const Annotation& annotation);

  // Bind name of @template tags.
  void ProcessTemplateTags(const Annotation& annotation);

  std::vector<const TypeParameter*> ProcessTypeParameterNames(
      const std::vector<const ast::Node*>& type_names);

  void ProcessVariableDeclaration(VariableKind kind,
                                  const ast::Node& node,
                                  const Annotation& annotated);

  // Returns class of |node| if known.
  const Class* TryClassOfPrototype(const ast::Node& node) const;

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

  void VisitInternal(const ast::ParameterList& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::ReferenceExpression& syntax,
                     const ast::Node& node) final;

  // Statement
  void VisitInternal(const ast::BlockStatement& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::CatchClause& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::ConstStatement& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::LetStatement& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::VarStatement& syntax,
                     const ast::Node& node) final;

  // Types
  void VisitInternal(const ast::TypeName& syntax, const ast::Node& node) final;

  std::vector<const ast::Node*> ancestors_;

  Environment* environment_ = nullptr;

  const std::unique_ptr<Environment> global_environment_;

  // Pass |VariabkeKind| to descendants.
  VariableKind variable_kind_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_NAME_RESOLVER_H_

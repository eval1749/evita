// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/type_resolver.h"

#include "base/logging.h"
#include "joana/analyzer/annotation_compiler.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/type_annotation_transformer.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"

namespace joana {
namespace analyzer {

//
// TypeResolver
//
TypeResolver::TypeResolver(Context* context) : Pass(context) {}
TypeResolver::~TypeResolver() = default;

// The entry point
void TypeResolver::RunOn(const ast::Node& node) {
  Visit(node);
}

const Type* TypeResolver::ComputeClassType(const ast::Node& node) const {
  if (node.Is<ast::ComputedMemberExpression>()) {
    const auto& member = ast::ComputedMemberExpression::ExpressionOf(node);
    if (!member.Is<ast::MemberExpression>())
      return nullptr;
    if (ast::MemberExpression::NameOf(member) != ast::TokenKind::Prototype)
      return nullptr;
    return context().TryTypeOf(ast::MemberExpression::ContainerOf(member));
  }
  if (node.Is<ast::MemberExpression>()) {
    const auto& member = ast::MemberExpression::ContainerOf(node);
    if (!member.Is<ast::MemberExpression>())
      return nullptr;
    if (ast::MemberExpression::NameOf(member) != ast::TokenKind::Prototype)
      return nullptr;
    return context().TryTypeOf(ast::MemberExpression::ContainerOf(member));
  }
  return nullptr;
}

const Type* TypeResolver::ComputeType(const Annotation& annotation,
                                      const ast::Node& node,
                                      const Type* maybe_this_type) {
  TypeAnnotationTransformer transformer(&context(), annotation, node,
                                        maybe_this_type);
  return transformer.Compile();
}

void TypeResolver::ProcessArrayBinding(const ast::Node& node,
                                       const Type& type) {
  NOTREACHED() << "NYI ProcessArrayBinding" << node << ' ' << type;
}

void TypeResolver::ProcessAnnotation(const ast::Node& node,
                                     const Annotation& annotation,
                                     const Type* maybe_this_type) {
  const auto* const type = ComputeType(annotation, node, maybe_this_type);
  if (!type)
    return;
  RegisterType(node, *type);
}

void TypeResolver::ProcessAssignment(const ast::Node& node,
                                     const Annotation& annotation) {
  DCHECK_EQ(node, ast::SyntaxCode::AssignmentExpression);
  const auto& lhs = ast::AssignmentExpression::LeftHandSideOf(node);
  const auto* const class_type = ComputeClassType(lhs);
  const auto* const type = ComputeType(annotation, node, class_type);
  if (!type)
    return;
  RegisterType(lhs, *type);
}

void TypeResolver::ProcessBinding(const ast::Node& node, const Type& type) {
  if (node.Is<ast::BindingNameElement>())
    return RegisterType(node, type);
  if (node.Is<ast::ArrayBindingPattern>())
    return ProcessArrayBinding(node, type);
  if (node.Is<ast::ObjectBindingPattern>())
    return ProcessObjectBinding(node, type);
  AddError(node, ErrorCode::TYPE_RESOLVER_UNEXPECT_BINDING);
}

void TypeResolver::ProcessObjectBinding(const ast::Node& node,
                                        const Type& type) {
  NOTREACHED() << "NYI ProcessObjectBinding" << node << ' ' << type;
}

void TypeResolver::ProcessVariableDeclaration(const ast::Node& node,
                                              const Annotation& annotation) {
  for (const auto& binding : ast::NodeTraversal::ChildNodesOf(node)) {
    auto* const value = SingleVariableValueOf(binding);
    const auto* class_type =
        value && value->Is<Class>()
            ? &type_factory().NewClassType(&value->As<Class>())
            : nullptr;
    const auto* type = ComputeType(annotation, binding, class_type);
    if (!type)
      continue;
    RegisterType(binding, *type);
  }
}

void TypeResolver::RegisterType(const ast::Node& node, const Type& type) {
  context().RegisterType(node, type);
}

Value* TypeResolver::SingleVariableValueOf(const ast::Node& node) const {
  const auto& variable = context().ValueOf(node).As<Variable>();
  if (variable.assignments().size() != 1)
    return nullptr;
  const auto& assignment = *variable.assignments().front();
  if (assignment.Is<ast::JsDocDocument>())
    return &context().ValueOf(assignment);
  if (assignment.Is<ast::BindingNameElement>()) {
    return context().TryValueOf(
        ast::BindingNameElement::InitializerOf(assignment));
  }
  return nullptr;
}

// |ast::SyntaxVisitor| members
void TypeResolver::VisitDefault(const ast::Node& node) {
  for (const ast::Node& child : ast::NodeTraversal::ChildNodesOf(node))
    Visit(child);
}

void TypeResolver::VisitInternal(const ast::Annotation& syntax,
                                 const ast::Node& node) {
  const auto& annotated = ast::Annotation::AnnotatedOf(node);
  Visit(annotated);
  const auto& document = ast::Annotation::DocumentOf(node);
  const auto annotation = Annotation::Compiler().Compile(document, node);
  if (annotated.Is<ast::Function>())
    return ProcessAnnotation(annotated, annotation, nullptr);
  if (annotated.Is<ast::GroupExpression>())
    return ProcessAnnotation(annotated, annotation, nullptr);
  if (annotated.syntax().Is<ast::VariableDeclaration>())
    return ProcessVariableDeclaration(annotated, annotation);
  if (!annotated.Is<ast::ExpressionStatement>())
    return;
  const auto& expression = ast::ExpressionStatement::ExpressionOf(annotated);
  if (expression.Is<ast::AssignmentExpression>())
    return ProcessAssignment(expression, annotation);
  const auto* const class_type = ComputeClassType(expression);
  return ProcessAnnotation(expression, annotation, class_type);
}

void TypeResolver::VisitInternal(const ast::Class& syntax,
                                 const ast::Node& node) {
  auto& class_value = context().ValueOf(node).As<Class>();
  const auto& class_type = type_factory().NewClassType(&class_value);
  for (const auto& child :
       ast::NodeTraversal::ChildNodesOf(ast::Class::BodyOf(node))) {
    if (!child.Is<ast::Annotation>())
      continue;
    const auto& document = ast::Annotation::DocumentOf(child);
    const auto& member = ast::Annotation::AnnotatedOf(child);
    if (!member.Is<ast::Method>())
      continue;
    const auto annotation = Annotation::Compiler().Compile(document, node);
    const auto* const this_type =
        ast::Method::IsStatic(member) ? nullptr : &class_type;
    ProcessAnnotation(member, annotation, this_type);
  }
}

}  // namespace analyzer
}  // namespace joana

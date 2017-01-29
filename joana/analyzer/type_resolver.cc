// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <utility>

#include "joana/analyzer/type_resolver.h"

#include "base/logging.h"
#include "joana/analyzer/annotation_compiler.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/type_annotation_transformer.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

namespace {

bool IsClass(const Value& value) {
  if (value.Is<Class>())
    return value.As<Class>().kind() == ClassKind::Class;
  if (value.Is<ConstructedClass>())
    return IsClass(value.As<ConstructedClass>());
  NOTREACHED() << "Expect Class or ConstructedClass " << value;
  return false;
}

}  // namespace

//
// TypeResolver
//
TypeResolver::TypeResolver(Context* context) : Pass(context) {}
TypeResolver::~TypeResolver() = default;

// The entry point
void TypeResolver::RunOn(const ast::Node& node) {
  Visit(node);
}

// private
std::vector<Value*> TypeResolver::ComputeClassHeritage(
    const Annotation& annotation,
    const ast::Node& node) {
  const auto is_class = IsClass(context().ValueOf(node));
  std::vector<std::pair<const ast::Node*, Value*>> references;
  std::vector<Value*> class_list;
  if (node.Is<ast::Class>()) {
    const auto& reference = ast::Class::HeritageOf(node);
    if (!reference.Is<ast::ElisionExpression>()) {
      auto* class_value = ResolveClass(reference);
      if (class_value && IsClass(*class_value)) {
        references.emplace_back(&reference, class_value);
        class_list.emplace_back(class_value);
      } else {
        AddError(reference, ErrorCode::TYPE_RESOLVER_EXPECT_CLASS);
      }
    }
  }
  for (const auto& extends_tag : annotation.extends_tags()) {
    const auto& reference = extends_tag->child_at(1);
    auto* const class_value = ResolveClass(reference);
    if (!class_value || is_class != IsClass(*class_value)) {
      AddError(reference, ErrorCode::TYPE_RESOLVER_EXPECT_CLASS);
      continue;
    }
    const auto& it =
        std::find_if(references.begin(), references.end(),
                     [&](const std::pair<const ast::Node*, Value*>& present) {
                       return present.second == class_value;
                     });
    if (it != references.end()) {
      AddError(*extends_tag, ErrorCode::TYPE_RESOLVER_MULTIPLE_OCCURRENCES,
               *it->first);
      continue;
    }
    if (node.Is<ast::Class>()) {
      AddError(*extends_tag, ErrorCode::TYPE_RESOLVER_UNEXPECT_EXTENDS, node);
      continue;
    }
    if (is_class && !class_list.empty()) {
      AddError(*extends_tag, ErrorCode::TYPE_RESOLVER_UNEXPECT_EXTENDS, node);
      continue;
    }
    references.emplace_back(&reference, class_value);
    class_list.emplace_back(class_value);
  }
  for (const auto& implements_tag : annotation.implements_tags()) {
    const auto& reference = implements_tag->child_at(1);
    auto* const class_value = ResolveClass(reference);
    if (!class_value || IsClass(*class_value)) {
      AddError(*implements_tag, ErrorCode::TYPE_RESOLVER_EXPECT_INTERFACE);
      continue;
    }
    const auto& it =
        std::find_if(references.begin(), references.end(),
                     [&](const std::pair<const ast::Node*, Value*>& present) {
                       return present.second == class_value;
                     });
    if (it != references.end()) {
      AddError(*implements_tag, ErrorCode::TYPE_RESOLVER_MULTIPLE_OCCURRENCES,
               *it->first);
      continue;
    }
    if (!is_class) {
      AddError(*implements_tag, ErrorCode::TYPE_RESOLVER_UNEXPECT_IMPLEMENTS,
               node);
      continue;
    }
    references.emplace_back(&reference, class_value);
    class_list.emplace_back(class_value);
  }
  DCHECK_EQ(class_list.size(), references.size());
  return class_list;
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

void TypeResolver::ProcessClass(const ast::Node& node,
                                const Annotation& annotation) {
  Visit(ast::Class::HeritageOf(node));
  auto& class_value = context().ValueOf(node).As<Class>();
  Value::Editor().SetClassHeritage(&class_value,
                                   ComputeClassHeritage(annotation, node));
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

Value* TypeResolver::ResolveClass(const ast::Node& node) {
  if (node.Is<ast::TypeName>()) {
    const auto* type = context().TryTypeOf(node);
    if (!type || !type->Is<ClassType>())
      return nullptr;
    return &type->As<ClassType>().value();
  }
  auto* value = context().TryValueOf(node);
  if (!value)
    return nullptr;
  if (value->Is<Class>() || value->Is<ConstructedClass>())
    return value;
  if (!value->Is<Variable>())
    return nullptr;
  const auto& variable = value->As<Variable>();
  if (variable.assignments().size() != 1)
    return nullptr;
  const auto& assignment = *variable.assignments().front();
  auto* assignment_value = context().TryValueOf(assignment);
  if (!assignment_value)
    return nullptr;
  if (assignment_value->Is<Class>())
    return assignment_value;
  if (assignment_value->Is<ConstructedClass>())
    return assignment_value;
  return nullptr;
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
  if (annotated.Is<ast::Class>())
    return ProcessClass(annotated, annotation);
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
  ProcessClass(node, Annotation());
}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <utility>

#include "joana/analyzer/environment_builder.h"

#include "base/auto_reset.h"
#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/properties.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/value_forward.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node* FindClassAnnotation(const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  for (const auto& member : ast::NodeTraversal::ChildNodesOf(document)) {
    if (member != ast::SyntaxCode::JsDocTag)
      continue;
    const auto& tag = ast::JsDocTag::NameOf(member);
    if (tag == ast::TokenKind::JsDocConstructor ||
        tag == ast::TokenKind::JsDocInterface ||
        tag == ast::TokenKind::JsDocRecord) {
      return &tag;
    }
  }
  return nullptr;
}

const ast::Node* FindTag(ast::TokenKind tag_name, const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  for (const auto& node : ast::NodeTraversal::ChildNodesOf(document)) {
    if (node != ast::SyntaxCode::JsDocTag)
      continue;
    if (ast::JsDocTag::NameOf(node) != tag_name)
      continue;
    return &node;
  }
  return nullptr;
}

// Returns true if |document| contains |@param|, |@return| or |@this| tag.
bool IsFunctionAnnotation(const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  for (const auto& member : ast::NodeTraversal::ChildNodesOf(document)) {
    if (member != ast::SyntaxCode::JsDocTag)
      continue;
    const auto& tag = ast::JsDocTag::NameOf(member);
    if (tag == ast::TokenKind::JsDocParam ||
        tag == ast::TokenKind::JsDocReturn ||
        tag == ast::TokenKind::JsDocThis) {
      return true;
    }
  }
  return false;
}

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

  void BindType(const ast::Node& name, const Type& type);
  void BindVariable(const ast::Node& name, Variable* value);
  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

 private:
  EnvironmentBuilder& builder_;
  LocalEnvironment* const outer_;
  const ast::Node& owner_;
  std::unordered_map<int, const Type*> type_map_;
  std::unordered_map<int, Variable*> value_map_;

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

void EnvironmentBuilder::LocalEnvironment::BindType(const ast::Node& name,
                                                    const Type& type) {
  const auto& result = type_map_.emplace(ast::Name::IdOf(name), &type);
  DCHECK(result.second);
}

void EnvironmentBuilder::LocalEnvironment::BindVariable(const ast::Node& name,
                                                        Variable* value) {
  const auto& result = value_map_.emplace(ast::Name::IdOf(name), value);
  DCHECK(result.second);
}

const Type* EnvironmentBuilder::LocalEnvironment::FindType(
    const ast::Node& name) const {
  const auto& it = type_map_.find(ast::Name::IdOf(name));
  return it == type_map_.end() ? nullptr : it->second;
}

Variable* EnvironmentBuilder::LocalEnvironment::FindVariable(
    const ast::Node& name) const {
  const auto& it = value_map_.find(ast::Name::IdOf(name));
  return it == value_map_.end() ? nullptr : it->second;
}

//
// EnvironmentBuilder
//
EnvironmentBuilder::EnvironmentBuilder(Context* context) : Pass(context) {}

EnvironmentBuilder::~EnvironmentBuilder() = default;

// The entry point. |node| is one of |ast::Externs|, |ast::Module| or
// |ast::Script|.
void EnvironmentBuilder::RunOn(const ast::Node& node) {
  auto& global_environment = context().global_environment();
  toplevel_environment_ =
      node == ast::SyntaxCode::Module
          ? &context().NewEnvironment(&global_environment, node)
          : &global_environment;
  Visit(node);
}

void EnvironmentBuilder::BindAsType(const ast::Node& name, Variable* variable) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    auto* const present = environment_->FindType(name);
    if (!present) {
      const auto& type = factory().NewTypeReference(variable);
      environment_->BindType(name, type);
      return;
    }
    if (present->Is<TypeReference>())
      return;
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  for (auto* runner = toplevel_environment_; runner; runner = runner->outer()) {
    auto* const present = runner->FindType(name);
    if (!present)
      continue;
    if (present->Is<TypeReference>())
      return;
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  const auto& type = factory().NewTypeReference(variable);
  toplevel_environment_->BindType(name, type);
}

Variable& EnvironmentBuilder::BindToVariable(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    for (auto* runner = environment_; runner; runner = runner->outer()) {
      if (auto* present = runner->FindVariable(name))
        return *present;
    }
    auto& variable = factory().NewVariable(name);
    environment_->BindVariable(name, &variable);
    return variable;
  }
  for (auto* runner = toplevel_environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindVariable(name))
      return *present;
  }
  // TODO(eval1749): Expose global "var" binding to global object.
  auto& variable = factory().NewVariable(name);
  toplevel_environment_->BindVariable(name, &variable);
  return variable;
}

const Type* EnvironmentBuilder::FindType(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    for (auto* runner = environment_; runner; runner = runner->outer()) {
      if (auto* present = runner->FindType(name))
        return present;
    }
  }
  for (auto* runner = toplevel_environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindType(name))
      return present;
  }
  return nullptr;
}

Variable* EnvironmentBuilder::FindVariable(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    for (auto* runner = environment_; runner; runner = runner->outer()) {
      if (auto* present = runner->FindVariable(name))
        return present;
    }
  }
  for (auto* runner = toplevel_environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindVariable(name))
      return present;
  }
  return nullptr;
}

// AST node handlers
void EnvironmentBuilder::ProcessAssignmentExpressionWithAnnotation(
    const ast::Node& node,
    const ast::Node& annotation) {
  if (ast::AssignmentExpression::OperatorOf(node) != ast::TokenKind::Equal) {
    AddError(annotation, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
    return;
  }
  const auto& lhs = ast::AssignmentExpression::LeftHandSideOf(node);
  auto* const value = context().TryValueOf(lhs);
  if (!value || !value->Is<ValueHolder>()) {
    // We've not known value of reference expression.
    return;
  }
  const auto& holder = value->As<ValueHolder>();
  if (holder.assignments().size() == 1)
    return;
  AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS);
}

void EnvironmentBuilder::ProcessMemberExpressionWithAnnotation(
    const ast::Node& node,
    const ast::Node& annotation) {
  auto* const value = context().TryValueOf(node);
  if (!value) {
    // We've not known value of member expression.
    return;
  }
  auto& property = value->As<ValueHolder>();
  if (!property.assignments().empty()) {
    AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS);
    return;
  }
  Value::Editor().AddAssignment(&property, node);
}

std::vector<const Type*> EnvironmentBuilder::ProcessTypeTemplate(
    const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  const auto* const template_tag =
      FindTag(ast::TokenKind::JsDocTemplate, document);
  if (!template_tag)
    return {};
  std::vector<const Type*> types;
  for (const auto& name : ast::JsDocTag::OperandsOf(*template_tag)) {
    if (const auto* present = environment_->FindType(name)) {
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      continue;
    }
    types.push_back(&factory().NewTypeParameter(name));
  }
  return std::move(types);
}

const Type& EnvironmentBuilder::ResolveTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (auto* present_type = FindType(name))
    return *present_type;

  if (auto* present = toplevel_environment_->FindVariable(name)) {
    const auto& type = factory().NewTypeReference(present);
    toplevel_environment_->BindType(name, type);
    return type;
  }

  auto& variable = factory().NewVariable(name);
  toplevel_environment_->BindVariable(name, &variable);
  const auto& type = factory().NewTypeReference(&variable);
  toplevel_environment_->BindType(name, type);
  return type;
}

Variable& EnvironmentBuilder::ResolveVariableName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (auto* present = FindVariable(name))
    return *present;
  // TODO(eval1749): Expose global "var" binding to global object.
  auto& variable = factory().NewVariable(name);
  toplevel_environment_->BindVariable(name, &variable);
  return variable;
}

//
// ast::NodeVisitor members
//
void EnvironmentBuilder::VisitDefault(const ast::Node& node) {
  VisitChildNodes(node);
}

// Binding elements
void EnvironmentBuilder::VisitInternal(const ast::BindingNameElement& syntax,
                                       const ast::Node& node) {
  VisitChildNodes(node);
  auto& variable = BindToVariable(ast::BindingNameElement::NameOf(node));
  Value::Editor().AddAssignment(&variable, node);
  context().RegisterValue(node, &variable);
  if (variable.assignments().size() == 1)
    return;
  AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS,
           *variable.assignments().front());
}

// Declarations
void EnvironmentBuilder::VisitInternal(const ast::Annotation& syntax,
                                       const ast::Node& node) {
  base::AutoReset<const ast::Node*> scope(&annotation_, &node);
  // Process annotated node before annotation to handle reference of class
  // name in constructor and parameter names for "@param".
  // Example:
  //  /**
  //   * @constructor
  //   * @return {!Foo} this repugnant though
  //   * @param {number} x
  //   */
  //  function Foo(x) {}
  Visit(ast::Annotation::AnnotatedOf(node));
  const auto& annotation = ast::Annotation::AnnotationOf(node);
  LocalEnvironment environment(this, annotation);
  const auto& type_parameters = ProcessTypeTemplate(annotation);
  for (const auto& type_parameter : type_parameters)
    environment_->BindType(type_parameter->node(), *type_parameter);
  Visit(annotation);
}

void EnvironmentBuilder::VisitInternal(const ast::Class& syntax,
                                       const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  auto& klass = factory().NewFunction(node);
  const auto& class_name = ast::Class::NameOf(node);
  if (class_name == ast::SyntaxCode::Name) {
    auto& variable = BindToVariable(class_name);
    if (!variable.assignments().empty()) {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS,
               *variable.assignments().front());
    }
    Value::Editor().AddAssignment(&variable, node);
    context().RegisterValue(node, &variable);
    BindAsType(class_name, &variable);
  } else {
    context().RegisterValue(node, &klass);
  }

  auto& prototype_property = factory().NewProperty(
      BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Prototype));
  Value::Editor().AddAssignment(&prototype_property, node);
  klass.properties().Add(&prototype_property);

  for (const auto& member :
       ast::NodeTraversal::ChildNodesOf(ast::Class::BodyOf(node))) {
    if (member != ast::SyntaxCode::Method) {
      AddError(member, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
      continue;
    }
    auto& properties =
        ast::Method::MethodKindOf(member) == ast::MethodKind::Static
            ? klass.properties()
            : prototype_property.properties();
    auto& property =
        factory().GetOrNewProperty(&properties, ast::Method::NameOf(member));
    auto& method = factory().NewFunction(member);
    context().RegisterValue(member, &method);
    if (!property.assignments().empty()) {
      AddError(member, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS,
               *property.assignments().front());
      continue;
    }
    Value::Editor().AddAssignment(&property, member);
  }
  VisitChildNodes(node);
}

void EnvironmentBuilder::VisitInternal(const ast::Function& syntax,
                                       const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& name = ast::Function::NameOf(node);
  auto& function = factory().NewFunction(node);
  if (name == ast::SyntaxCode::Name) {
    auto& variable = BindToVariable(name);
    if (!variable.assignments().empty()) {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS,
               *variable.assignments().front());
    }
    Value::Editor().AddAssignment(&variable, node);
    context().RegisterValue(node, &variable);
    if (annotation_ && ast::Annotation::AnnotatedOf(*annotation_) == node &&
        FindClassAnnotation(ast::Annotation::AnnotationOf(*annotation_)) !=
            nullptr) {
      BindAsType(name, &variable);
    }
  } else {
    context().RegisterValue(node, &function);
  }
  LocalEnvironment environment(this, node);
  VisitChildNodes(node);
}

void EnvironmentBuilder::VisitInternal(const ast::Method& syntax,
                                       const ast::Node& node) {
  // Methods are bound during processing class declaration.
  LocalEnvironment environment(this, node);
  VisitChildNodes(node);
}

// Expressions
void EnvironmentBuilder::VisitInternal(const ast::AssignmentExpression& syntax,
                                       const ast::Node& node) {
  VisitChildNodes(node);
  const auto& lhs = ast::AssignmentExpression::LeftHandSideOf(node);
  auto* const value = context().TryValueOf(lhs);
  if (!value) {
    // We've not known value of reference expression.
    return;
  }
  auto& holder = value->As<ValueHolder>();
  Value::Editor().AddAssignment(&holder, node);
}

void EnvironmentBuilder::VisitInternal(
    const ast::ComputedMemberExpression& syntax,
    const ast::Node& node) {
  VisitDefault(node);
  auto* const value = context().TryValueOf(
      ast::ComputedMemberExpression::MemberExpressionOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& key = ast::ComputedMemberExpression::ExpressionOf(node);
  if (!ast::IsKnownSymbol(key))
    return;
  auto& property = factory().GetOrNewProperty(&properties, key);
  context().RegisterValue(node, &property);
}

void EnvironmentBuilder::VisitInternal(const ast::MemberExpression& syntax,
                                       const ast::Node& node) {
  VisitDefault(node);
  auto* const value =
      context().TryValueOf(ast::MemberExpression::ExpressionOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& name = ast::MemberExpression::NameOf(node);
  auto& property = factory().GetOrNewProperty(&properties, name);
  context().RegisterValue(node, &property);
}

void EnvironmentBuilder::VisitInternal(const ast::ReferenceExpression& syntax,
                                       const ast::Node& node) {
  const auto& name = ast::ReferenceExpression::NameOf(node);
  if (ast::Name::IsKeyword(name))
    return;
  auto& variable = ResolveVariableName(name);
  context().RegisterValue(node, &variable);
}

// Statements
void EnvironmentBuilder::VisitInternal(const ast::BlockStatement& syntax,
                                       const ast::Node& node) {
  LocalEnvironment environment(this, node);
  VisitChildNodes(node);
}

void EnvironmentBuilder::VisitInternal(const ast::ExpressionStatement& syntax,
                                       const ast::Node& node) {
  VisitDefault(node);
  if (!annotation_ || ast::Annotation::AnnotatedOf(*annotation_) != node)
    return;
  const auto& expression = ast::ExpressionStatement::ExpressionOf(node);
  const auto& annotation = ast::Annotation::AnnotationOf(*annotation_);
  if (expression == ast::SyntaxCode::AssignmentExpression)
    return ProcessAssignmentExpressionWithAnnotation(expression, annotation);
  if (expression == ast::SyntaxCode::MemberExpression)
    return ProcessMemberExpressionWithAnnotation(expression, annotation);
  if (expression == ast::SyntaxCode::ComputedMemberExpression)
    return ProcessMemberExpressionWithAnnotation(expression, annotation);
  AddError(*annotation_, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
}

// Types
void EnvironmentBuilder::VisitInternal(const ast::TypeName& syntax,
                                       const ast::Node& node) {
  const auto& type = ResolveTypeName(ast::TypeName::NameOf(node));
  context().RegisterType(node, type);
}

}  // namespace analyzer
}  // namespace joana

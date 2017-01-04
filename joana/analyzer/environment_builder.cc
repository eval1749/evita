// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/analyzer/environment_builder.h"

#include "base/auto_reset.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/properties.h"
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
  const auto& result = value_map_.emplace(ast::Name::IdOf(name), value);
  DCHECK(result.second);
}

Value* EnvironmentBuilder::LocalEnvironment::Find(const ast::Node& name) const {
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
  auto& global_environment = factory().global_environment();
  toplevel_environment_ =
      node == ast::SyntaxCode::Module
          ? &factory().NewEnvironment(&global_environment, node)
          : &global_environment;
  SyntaxVisitor::Visit(node);
}

// Binding helpers
void EnvironmentBuilder::Bind(const ast::Node& name, Value* value) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      return;
    }
    environment_->Bind(name, value);
    return;
  }

  if (auto* present = toplevel_environment_->TryValueOf(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): We should bind to |Constructor| if |declaration| has
  // "@constructor" annotation.
  toplevel_environment_->Bind(name, value);
}

void EnvironmentBuilder::BindToVariable(const ast::Node& origin,
                                        const ast::Node& name_node) {
  const auto& name = NameOf(name_node);
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      if (present->node() == ast::SyntaxCode::VarStatement &&
          origin == ast::SyntaxCode::VarStatement) {
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
  if (auto* present = toplevel_environment_->TryValueOf(name)) {
    if (present->node() == ast::SyntaxCode::VarStatement &&
        origin == ast::SyntaxCode::VarStatement) {
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
void EnvironmentBuilder::ProcessAssignmentExpressionWithAnnotation(
    const ast::Node& node,
    const ast::Node& annotation) {
  if (ast::AssignmentExpression::OperatorOf(node) != ast::TokenKind::Equal)
    return;
}

void EnvironmentBuilder::ProcessMemberExpressionWithAnnotation(
    const ast::Node& node,
    const ast::Node& annotation) {
  const auto& expression = ast::MemberExpression::ExpressionOf(node);
  auto* const value = factory().TryValueOf(expression);
  if (!value || !value->Is<Object>()) {
    AddError(expression, ErrorCode::ENVIRONMENT_EXPECT_OBJECT);
    return;
  }
  auto& properties = value->As<Object>().properties();
  const auto& name = ast::MemberExpression::NameOf(node);
  auto* const property = properties.TryGet(name);
  if (!property || !property->assignments().empty()) {
    AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS);
    return;
  }
  property->AddAssignment(node);
  if (IsFunctionAnnotation(annotation)) {
    factory().NewFunction(node);
    return;
  }
  factory().NewVariable(node, name);
}

void EnvironmentBuilder::ProcessVariables(const ast::Node& statement) {
  variable_origin_ = &statement;
  VisitChildNodes(statement);
}

//
// ast::NodeVisitor members
//
void EnvironmentBuilder::VisitDefault(const ast::Node& node) {
  VisitChildNodes(node);
}

// Binding elements
void EnvironmentBuilder::Visit(const ast::BindingNameElement& syntax,
                               const ast::Node& node) {
  BindToVariable(*variable_origin_, node);
  VisitChildNodes(node);
}

// Declarations
void EnvironmentBuilder::Visit(const ast::Annotation& syntax,
                               const ast::Node& node) {
  base::AutoReset<const ast::Node*> scope(&annotation_, &node);
  VisitChildNodes(node);
}

void EnvironmentBuilder::Visit(const ast::Class& syntax,
                               const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  auto& klass = factory().NewClass(node).As<Class>();
  if (ast::Class::NameOf(node) == ast::SyntaxCode::Name)
    Bind(ast::Class::NameOf(node), &klass);
  for (const auto& member :
       ast::NodeTraversal::ChildNodesOf(ast::Class::BodyOf(node))) {
    if (member != ast::SyntaxCode::Method) {
      AddError(member, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
      continue;
    }
    auto& properties =
        ast::Method::MethodKindOf(member) == ast::MethodKind::Static
            ? klass.properties()
            : klass.prototype().properties();
    auto& property =
        factory().GetOrNewProperty(&properties, ast::Method::NameOf(member));
    factory().NewMethod(member, &klass);
    if (!property.assignments().empty()) {
      AddError(member, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS,
               *property.assignments().front());
      continue;
    }
    property.AddAssignment(member);
  }
  VisitChildNodes(node);
}

void EnvironmentBuilder::Visit(const ast::Function& syntax,
                               const ast::Node& node) {
  const auto& name = ast::Function::NameOf(node);
  const auto* const class_tag =
      annotation_
          ? FindClassAnnotation(ast::Annotation::AnnotationOf(*annotation_))
          : nullptr;
  if (class_tag) {
    auto& klass = factory().NewClass(node, *class_tag);
    if (name == ast::SyntaxCode::Name)
      Bind(name, &klass);
  } else {
    // TODO(eval1749): Report warning for toplevel anonymous class
    auto& function = factory().NewFunction(node);
    if (name == ast::SyntaxCode::Name)
      Bind(name, &function);
  }
  LocalEnvironment environment(this, node);
  variable_origin_ = &node;
  VisitChildNodes(node);
}

void EnvironmentBuilder::Visit(const ast::Method& syntax,
                               const ast::Node& node) {
  // Methods are bound during processing class declaration.
  LocalEnvironment environment(this, node);
  variable_origin_ = &node;
  VisitChildNodes(node);
}

// Expressions
void EnvironmentBuilder::Visit(const ast::MemberExpression& syntax,
                               const ast::Node& node) {
  VisitDefault(node);
  auto* const value =
      factory().TryValueOf(ast::MemberExpression::ExpressionOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& name = ast::MemberExpression::NameOf(node);
  // TODO(eval1749): We should not create property if this |MemberExpression|
  // is right and side.
  auto& property = factory().GetOrNewProperty(&properties, name);
  if (property.assignments().empty())
    return;
  auto* const property_value =
      factory().TryValueOf(*property.assignments().front());
  if (!property_value)
    return;
  factory().RegisterValue(node, property_value);
}

void EnvironmentBuilder::Visit(const ast::ReferenceExpression& syntax,
                               const ast::Node& node) {
  const auto& name = ast::ReferenceExpression::NameOf(node);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->Find(name)) {
      factory().RegisterValue(node, present);
      return;
    }
  }
  auto* present = toplevel_environment_->TryValueOf(name);
  if (!present)
    return;
  factory().RegisterValue(node, present);
}

// Statements
void EnvironmentBuilder::Visit(const ast::BlockStatement& syntax,
                               const ast::Node& node) {
  LocalEnvironment environment(this, node);
  VisitChildNodes(node);
}

void EnvironmentBuilder::Visit(const ast::ConstStatement& syntax,
                               const ast::Node& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::Visit(const ast::ExpressionStatement& syntax,
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
  AddError(*annotation_, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
}

void EnvironmentBuilder::Visit(const ast::LetStatement& syntax,
                               const ast::Node& node) {
  ProcessVariables(node);
}

void EnvironmentBuilder::Visit(const ast::VarStatement& syntax,
                               const ast::Node& node) {
  ProcessVariables(node);
}

}  // namespace analyzer
}  // namespace joana

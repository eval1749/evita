// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
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
#include "joana/analyzer/type_factory.h"
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

ClassKind ClassKindOf(const ast::Node& node) {
  DCHECK_EQ(node, ast::SyntaxCode::JsDocTag);
  const auto& name = ast::JsDocTag::NameOf(node);
  if (name == ast::TokenKind::JsDocConstructor)
    return ClassKind::Normal;
  if (name == ast::TokenKind::JsDocInterface)
    return ClassKind::Interface;
  if (name == ast::TokenKind::JsDocRecord)
    return ClassKind::Record;
  NOTREACHED() << "Expect @constructor, @Interface or @record " << node;
  return ClassKind::Normal;
}

bool IsClassTag(const ast::Node& node) {
  if (!node.Is<ast::JsDocTag>())
    return false;
  const auto& name = ast::JsDocTag::NameOf(node);
  return name == ast::TokenKind::JsDocConstructor ||
         name == ast::TokenKind::JsDocInterface ||
         name == ast::TokenKind::JsDocRecord;
}

bool IsMemberExpression(const ast::Node& node) {
  return node == ast::SyntaxCode::MemberExpression ||
         node == ast::SyntaxCode::ComputedMemberExpression;
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
  std::pair<const ast::Node*, const Type*> FindNameAndType(
      const ast::Node& name) const;
  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

 private:
  EnvironmentBuilder& builder_;
  LocalEnvironment* const outer_;
  const ast::Node& owner_;
  std::unordered_map<int, std::pair<const ast::Node*, const Type*>> type_map_;
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
  const auto name_id = ast::Name::IdOf(name);
  const auto& result = type_map_.emplace(name_id, std::make_pair(&name, &type));
  DCHECK(result.second);
}

void EnvironmentBuilder::LocalEnvironment::BindVariable(const ast::Node& name,
                                                        Variable* value) {
  const auto& result = value_map_.emplace(ast::Name::IdOf(name), value);
  DCHECK(result.second);
}

std::pair<const ast::Node*, const Type*>
EnvironmentBuilder::LocalEnvironment::FindNameAndType(
    const ast::Node& name) const {
  const auto& it = type_map_.find(ast::Name::IdOf(name));
  return it == type_map_.end()
             ? std::pair<const ast::Node*, const Type*>(nullptr, nullptr)
             : it->second;
}

const Type* EnvironmentBuilder::LocalEnvironment::FindType(
    const ast::Node& name) const {
  const auto& it = type_map_.find(ast::Name::IdOf(name));
  return it == type_map_.end() ? nullptr : it->second.second;
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

void EnvironmentBuilder::BindAsType(const ast::Node& name, const Type& type) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (environment_) {
    const auto& present = environment_->FindNameAndType(name);
    const auto* present_name = present.first;
    const auto* present_type = present.second;
    if (!present_type) {
      environment_->BindType(name, type);
      return;
    }
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES, *present_name);
    return;
  }
  for (auto* runner = toplevel_environment_; runner; runner = runner->outer()) {
    const auto& present = runner->FindNameAndType(name);
    const auto* present_name = present.first;
    const auto* present_type = present.second;
    if (!present_type)
      continue;
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES, *present_name);
    return;
  }
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

void EnvironmentBuilder::ProcessFunction(const ast::Node& node,
                                         const ast::Node* maybe_document) {
  DCHECK_EQ(node, ast::SyntaxCode::Function);
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& name = ast::Function::NameOf(node);
  auto& function = factory().NewFunction(node);
  if (name == ast::SyntaxCode::Name) {
    auto& variable = BindToVariable(name);
    if (!variable.assignments().empty()) {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *variable.assignments().front());
    }
    Value::Editor().AddAssignment(&variable, node);
    if (!maybe_document || !RegisterConstructorIfNeeded(node, *maybe_document))
      context().RegisterValue(node, &function);
  } else {
    context().RegisterValue(node, &function);
  }

  LocalEnvironment environment(this, node);
  VisitChildNodes(node);

  if (!maybe_document)
    return;
  Visit(*maybe_document);
}

bool EnvironmentBuilder::RegisterConstructorIfNeeded(
    const ast::Node& node,
    const ast::Node& document) {
  const ast::Node* class_tag = nullptr;
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(document)) {
    if (!IsClassTag(child))
      continue;
    if (!class_tag) {
      class_tag = &child;
      continue;
    }
    AddError(child, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES, *class_tag);
  }
  if (!class_tag)
    return false;
  auto& class_value = factory().NewClass(node, ClassKindOf(*class_tag));
  auto& class_type = type_factory().NewClassType(&class_value);
  context().RegisterType(node, class_type);
  context().RegisterValue(node, &class_value);
  BindAsType(ast::Function::NameOf(node), class_type);
  return true;
}

// AST node handlers
Variable& EnvironmentBuilder::ResolveVariableName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (auto* present = FindVariable(name))
    return *present;
  // TODO(eval1749): Expose global "var" binding to global object.
  auto& variable = factory().NewVariable(name);
  toplevel_environment_->BindVariable(name, &variable);
  return variable;
}

void EnvironmentBuilder::VisitChildNodes(const ast::Node& node) {
  ancestors_.push_back(&node);
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(node))
    Visit(child);
  ancestors_.pop_back();
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
  VisitDefault(node);
  auto& variable = BindToVariable(ast::BindingNameElement::NameOf(node));
  Value::Editor().AddAssignment(&variable, node);
  context().RegisterValue(node, &variable);
  if (variable.assignments().size() == 1)
    return;
  AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
           *variable.assignments().front());
}

// Declarations
void EnvironmentBuilder::VisitInternal(const ast::Annotation& syntax,
                                       const ast::Node& node) {
  // Process annotated node before annotation to handle reference of class
  // name in constructor and parameter names for "@param".
  // Example:
  //  /**
  //   * @constructor
  //   * @return {!Foo} this repugnant though
  //   * @param {number} x
  //   */
  //  function Foo(x) {}
  const auto& annotated = ast::Annotation::AnnotatedOf(node);
  const auto& document = ast::Annotation::AnnotationOf(node);
  if (annotated.Is<ast::Class>()) {
    Visit(annotated);
    Visit(document);
    return;
  }
  if (annotated.Is<ast::Function>())
    return ProcessFunction(annotated, &document);
  if (annotated.syntax().Is<ast::VariableDeclaration>()) {
    VisitChildNodes(node);
    return;
  }
  if (!annotated.Is<ast::ExpressionStatement>()) {
    AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
    return;
  }
  const auto& expression = ast::ExpressionStatement::ExpressionOf(annotated);
  if (expression.Is<ast::AssignmentExpression>()) {
    const auto& lhs = ast::AssignmentExpression::LeftHandSideOf(expression);
    const auto& rhs = ast::AssignmentExpression::RightHandSideOf(expression);
    if (!IsMemberExpression(lhs)) {
      AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
      Visit(annotated);
      return;
    }
    if (rhs.Is<ast::Class>()) {
      Visit(rhs);
      Visit(document);
      return;
    }
    if (rhs.Is<ast::Function>()) {
      ProcessFunction(rhs, &document);
      Visit(lhs);
      return;
    }
    Visit(annotated);
    Visit(document);
    return;
  }

  if (expression.Is<ast::MemberExpression>()) {
    Visit(expression);
    Visit(document);
    return;
  }
  if (expression.Is<ast::ComputedMemberExpression>()) {
    Visit(expression);
    Visit(document);
    return;
  }
  VisitDefault(node);
  AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
}

void EnvironmentBuilder::VisitInternal(const ast::Class& syntax,
                                       const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  // TODO(eval1749): We should check annotation for class to check
  // @interface and @record.
  const auto class_kind = ClassKind::Normal;
  auto& class_value = factory().NewClass(node, class_kind);
  const auto& class_type = type_factory().NewClassType(&class_value);
  context().RegisterType(node, class_type);
  context().RegisterValue(node, &class_value);

  for (const auto& child :
       ast::NodeTraversal::ChildNodesOf(ast::Class::BodyOf(node))) {
    const auto& member = child.Is<ast::Annotation>()
                             ? ast::Annotation::AnnotatedOf(child)
                             : child;

    if (member != ast::SyntaxCode::Method) {
      AddError(member, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
      continue;
    }

    const auto& method_name = ast::Method::NameOf(member);

    // constructor can not be static and async/generator.
    if (method_name == ast::TokenKind::Constructor) {
      if (ast::Method::MethodKindOf(member) != ast::MethodKind::NonStatic ||
          ast::Method::FunctionKindOf(member) != ast::FunctionKind::Normal) {
        AddError(member, ErrorCode::ENVIRONMENT_INVALID_CONSTRUCTOR);
      }
    }

    // Check multiple occurrence
    const auto& it =
        std::find_if(class_value.methods().begin(), class_value.methods().end(),
                     [&](const Function& present) {
                       return present.node() == ast::Name::KindOf(method_name);
                     });
    if (it != class_value.methods().end())
      AddError(member, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               (*it).node());

    auto& method = factory().NewFunction(member);
    context().RegisterValue(member, &method);
    Value::Editor().AddMethod(&class_value, &method);

    LocalEnvironment environment(this, node);
    VisitDefault(member);

    if (!child.Is<ast::Annotation>())
      continue;
    VisitDefault(ast::Annotation::AnnotationOf(child));
  }

  const auto& class_name = ast::Class::NameOf(node);
  if (class_name != ast::SyntaxCode::Name)
    return;

  BindAsType(class_name, class_type);

  // Set variable
  auto& variable = BindToVariable(class_name);
  if (!variable.assignments().empty()) {
    AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             *variable.assignments().front());
  }
  Value::Editor().AddAssignment(&variable, node);

  // Populate class properties and prototype properties with methods.
  auto& prototype_property = factory().NewProperty(
      BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Prototype));
  Value::Editor().AddAssignment(&prototype_property, node);

  for (const auto& method : class_value.methods()) {
    const auto& member = method.node();
    auto& properties =
        ast::Method::MethodKindOf(member) == ast::MethodKind::Static
            ? variable.properties()
            : prototype_property.properties();
    auto& property =
        factory().GetOrNewProperty(&properties, ast::Method::NameOf(member));
    if (!property.assignments().empty()) {
      AddError(member, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *property.assignments().front());
      continue;
    }
    Value::Editor().AddAssignment(&property, member);
  }
}

void EnvironmentBuilder::VisitInternal(const ast::Function& syntax,
                                       const ast::Node& node) {
  ProcessFunction(node, nullptr);
}

void EnvironmentBuilder::VisitInternal(const ast::Method& syntax,
                                       const ast::Node& node) {
  NOTREACHED() << "Method should be processed in Class " << node;
}

// Expressions
void EnvironmentBuilder::VisitInternal(const ast::AssignmentExpression& syntax,
                                       const ast::Node& node) {
  VisitDefault(node);
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
  VisitDefault(node);
}

// Types
void EnvironmentBuilder::VisitInternal(const ast::TypeName& syntax,
                                       const ast::Node& node) {
  const auto* type = FindType(ast::TypeName::NameOf(node));
  if (!type)
    return;
  context().RegisterType(node, *type);
}

}  // namespace analyzer
}  // namespace joana

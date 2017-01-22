// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "joana/analyzer/name_resolver.h"

#include "base/auto_reset.h"
#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/context.h"
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
#include "joana/base/iterator_utils.h"

namespace joana {
namespace analyzer {

namespace {

ClassKind ClassKindOf(const ast::Node& node) {
  DCHECK_EQ(node, ast::SyntaxCode::JsDocTag);
  const auto& name = ast::JsDocTag::NameOf(node);
  if (name == ast::TokenKind::AtConstructor)
    return ClassKind::Class;
  if (name == ast::TokenKind::AtInterface)
    return ClassKind::Interface;
  if (name == ast::TokenKind::AtRecord)
    return ClassKind::Record;
  NOTREACHED() << "Expect @constructor, @Interface or @record " << node;
  return ClassKind::Class;
}

bool IsClassTag(const ast::Node& node) {
  if (!node.Is<ast::JsDocTag>())
    return false;
  const auto& name = ast::JsDocTag::NameOf(node);
  return name == ast::TokenKind::AtConstructor ||
         name == ast::TokenKind::AtInterface ||
         name == ast::TokenKind::AtRecord;
}

bool IsKindTag(const ast::Node& node) {
  if (!node.Is<ast::JsDocTag>())
    return false;
  const auto& name = ast::JsDocTag::NameOf(node);
  return name == ast::TokenKind::AtConstructor ||
         name == ast::TokenKind::AtInterface ||
         name == ast::TokenKind::AtRecord || name == ast::TokenKind::AtTypeDef;
}

bool IsMemberExpression(const ast::Node& node) {
  return node == ast::SyntaxCode::MemberExpression ||
         node == ast::SyntaxCode::ComputedMemberExpression;
}

// constructor can not be async/generator.
bool IsValidConstructor(const ast::Node& method) {
  DCHECK_EQ(ast::Method::NameOf(method), ast::TokenKind::Constructor);
  if (ast::Method::MethodKindOf(method) == ast::MethodKind::Static)
    return true;
  return ast::Method::FunctionKindOf(method) == ast::FunctionKind::Normal;
}

}  // namespace

//
// NameResolver::Environment
//
class NameResolver::Environment final {
 public:
  explicit Environment(NameResolver* resolver);
  ~Environment();

  const Environment* outer() const { return outer_; }
  Environment* outer() { return outer_; }

  void AddForwardReferencedType(const ast::Node& node);
  void AddForwardReferencedVariable(const ast::Node& node);

  void BindType(const ast::Node& name, const Type& type);
  void BindVariable(const ast::Node& name, Variable* value);
  std::pair<const ast::Node*, const Type*> FindNameAndType(
      const ast::Node& name) const;
  const Type* FindType(const ast::Node& name) const;
  Variable* FindVariable(const ast::Node& name) const;

  static std::unique_ptr<Environment> NewGlobalEnvironment(
      NameResolver* resolver);

 private:
  void ResolveForwardReferences();

  std::vector<const ast::Node*> forward_referenced_types_;
  std::vector<const ast::Node*> forward_referenced_variables_;
  Environment* const outer_;
  NameResolver& resolver_;
  std::unordered_map<int, std::pair<const ast::Node*, const Type*>> type_map_;
  std::unordered_map<int, Variable*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Environment);
};

NameResolver::Environment::Environment(NameResolver* resolver)
    : outer_(resolver->environment_), resolver_(*resolver) {
  resolver_.environment_ = this;
}

NameResolver::Environment::~Environment() {
  ResolveForwardReferences();
  resolver_.environment_ = outer_;
}

void NameResolver::Environment::AddForwardReferencedType(
    const ast::Node& node) {
  DCHECK_EQ(node, ast::SyntaxCode::TypeName);
  forward_referenced_types_.push_back(&node);
}

void NameResolver::Environment::AddForwardReferencedVariable(
    const ast::Node& node) {
  DCHECK_EQ(node, ast::SyntaxCode::ReferenceExpression);
  forward_referenced_variables_.push_back(&node);
}

void NameResolver::Environment::BindType(const ast::Node& name,
                                         const Type& type) {
  const auto name_id = ast::Name::IdOf(name);
  const auto& result = type_map_.emplace(name_id, std::make_pair(&name, &type));
  DCHECK(result.second);
}

void NameResolver::Environment::BindVariable(const ast::Node& name,
                                             Variable* value) {
  const auto& result = value_map_.emplace(ast::Name::IdOf(name), value);
  DCHECK(result.second);
}

std::pair<const ast::Node*, const Type*>
NameResolver::Environment::FindNameAndType(const ast::Node& name) const {
  const auto& it = type_map_.find(ast::Name::IdOf(name));
  return it == type_map_.end()
             ? std::pair<const ast::Node*, const Type*>(nullptr, nullptr)
             : it->second;
}

const Type* NameResolver::Environment::FindType(const ast::Node& name) const {
  const auto& it = type_map_.find(ast::Name::IdOf(name));
  return it == type_map_.end() ? nullptr : it->second.second;
}

Variable* NameResolver::Environment::FindVariable(const ast::Node& name) const {
  const auto& it = value_map_.find(ast::Name::IdOf(name));
  return it == value_map_.end() ? nullptr : it->second;
}

// static
std::unique_ptr<NameResolver::Environment>
NameResolver::Environment::NewGlobalEnvironment(NameResolver* resolver) {
  auto environment = std::make_unique<Environment>(resolver);
  // Initialize primitive types
  for (const auto id : BuiltInWorld::GetInstance()->primitive_types()) {
    const auto& name = BuiltInWorld::GetInstance()->NameOf(id);
    const auto& type = resolver->type_factory().NewPrimitiveType(id);
    resolver->context().RegisterType(name, type);
    environment->BindType(name, type);
  }

  // Install Global Object
  {
    const auto& name =
        BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Global);
    auto& variable = resolver->factory().NewVariable(VariableKind::Const, name);
    Value::Editor().AddAssignment(&variable, name);
    environment->BindVariable(name, &variable);
  }
  return std::move(environment);
}

void NameResolver::Environment::ResolveForwardReferences() {
  for (const auto& node : ReferenceRangeOf(forward_referenced_variables_)) {
    auto* const present = FindVariable(ast::ReferenceExpression::NameOf(node));
    if (present) {
      resolver_.context().RegisterValue(node, present);
      continue;
    }
    if (outer_) {
      outer_->AddForwardReferencedVariable(node);
      continue;
    }
    resolver_.AddError(node, ErrorCode::ENVIRONMENT_UNDEFINED_VARIABLE);
  }
  for (const auto& node : ReferenceRangeOf(forward_referenced_types_)) {
    const auto* const present = FindType(ast::TypeName::NameOf(node));
    if (present) {
      resolver_.context().RegisterType(node, *present);
      continue;
    }
    if (outer_) {
      outer_->AddForwardReferencedType(node);
      continue;
    }
    resolver_.AddError(node, ErrorCode::ENVIRONMENT_UNDEFINED_TYPE);
  }
}

//
// NameResolver
//
NameResolver::NameResolver(Context* context)
    : Pass(context),
      global_environment_(std::move(Environment::NewGlobalEnvironment(this))),
      variable_kind_(VariableKind::Function) {
  factory().ResetValueId();
}

NameResolver::~NameResolver() = default;

// The entry point. |node| is one of |ast::Externs|, |ast::Module| or
// |ast::Script|.
void NameResolver::RunOn(const ast::Node& node) {
  if (node.Is<ast::Module>()) {
    Environment toplevel_environment(this);
    Visit(node);
    return;
  }
  Visit(node);
}

void NameResolver::BindType(const ast::Node& name, const Type& type) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto& present = environment_->FindNameAndType(name);
  const auto* present_name = present.first;
  const auto* present_type = present.second;
  if (!present_type) {
    environment_->BindType(name, type);
    return;
  }
  AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES, *present_name);
}

void NameResolver::BindTypeParameters(const Class& class_value) {
  for (const auto& parameter : class_value.parameters())
    BindType(parameter.name(), parameter);
}

const ast::Node* NameResolver::ClassifyDocument(const ast::Node& document) {
  const ast::Node* kind_tag = nullptr;
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(document)) {
    if (!IsKindTag(child))
      continue;
    if (!kind_tag) {
      kind_tag = &child;
      continue;
    }
    AddError(child, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES, *kind_tag);
  }
  return kind_tag;
}

const Type* NameResolver::FindType(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindType(name))
      return present;
  }
  return nullptr;
}

Variable& NameResolver::BindVariable(VariableKind kind, const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (auto* present = environment_->FindVariable(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             present->node());
    return *present;
  }
  // TODO(eval1749): Expose global "var" binding to global object.
  auto& variable = factory().NewVariable(kind, name);
  environment_->BindVariable(name, &variable);
  return variable;
}

Variable* NameResolver::FindVariable(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindVariable(name))
      return present;
  }
  return nullptr;
}

void NameResolver::ProcessClass(const ast::Node& node,
                                const ast::Node* maybe_document) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  // TODO(eval1749): We should check annotation for class to check
  // @interface and @record.
  const auto& class_name = ast::Class::NameOf(node);

  const auto* const class_tag =
      maybe_document ? ProcessClassTag(*maybe_document) : nullptr;

  const auto& type_parameters = maybe_document
                                    ? ProcessTemplateTag(*maybe_document)
                                    : std::vector<const TypeParameter*>();

  const auto class_kind =
      class_tag ? ClassKindOf(*class_tag) : ClassKind::Class;

  auto& class_value = factory().NewClass(node, class_kind, type_parameters);
  context().RegisterValue(node, &class_value);
  if (class_name.Is<ast::Name>())
    BindType(class_name, type_factory().NewClassType(&class_value));

  // Class wide template parameters.
  Environment environment(this);
  BindTypeParameters(class_value);
  if (maybe_document)
    Visit(*maybe_document);

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
    if (method_name == ast::TokenKind::Constructor &&
        !IsValidConstructor(member)) {
      AddError(member, ErrorCode::ENVIRONMENT_INVALID_CONSTRUCTOR);
    }

    // Check multiple occurrence
    const auto& it =
        std::find_if(class_value.methods().begin(), class_value.methods().end(),
                     [&](const Function& present) {
                       return present.node() == ast::Name::KindOf(method_name);
                     });
    if (it != class_value.methods().end()) {
      AddError(member, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               (*it).node());
    }

    auto& method = factory().NewFunction(member);
    context().RegisterValue(member, &method);
    Value::Editor().AddMethod(&class_value, &method);

    Environment environment(this);
    BindTypeParameters(class_value);
    VisitDefault(member);

    if (!child.Is<ast::Annotation>())
      continue;
    VisitDefault(ast::Annotation::DocumentOf(child));
  }

  if (class_name != ast::SyntaxCode::Name)
    return;

  // Set variable
  auto& variable = BindVariable(VariableKind::Class, class_name);
  if (!variable.assignments().empty()) {
    AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             *variable.assignments().front());
  }
  Value::Editor().AddAssignment(&variable, node);

  // Populate class properties and prototype properties with methods.
  auto& prototype_property = factory().NewProperty(
      Visibility::Public,
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

const ast::Node* NameResolver::ProcessClassTag(const ast::Node& document) {
  const auto* const kind_tag = ClassifyDocument(document);
  if (!kind_tag)
    return kind_tag;
  if (!IsClassTag(*kind_tag)) {
    AddError(*kind_tag, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
    return nullptr;
  }
  return kind_tag;
}

void NameResolver::ProcessDocument(const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  Environment environment(this);
  for (const auto& type_parameter : ProcessTemplateTag(document))
    BindType(type_parameter->name(), *type_parameter);
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(document)) {
    if (!child.Is<ast::JsDocTag>())
      continue;
    if (ast::JsDocTag::NameOf(child) != ast::TokenKind::AtParam)
      continue;
    const auto& reference = child.child_at(2);
    if (!reference.Is<ast::ReferenceExpression>())
      continue;
    BindVariable(VariableKind::Parameter,
                 ast::ReferenceExpression::NameOf(reference));
  }
  Visit(document);
}

void NameResolver::ProcessFunction(const ast::Node& node,
                                   const ast::Node* maybe_document) {
  DCHECK_EQ(node, ast::SyntaxCode::Function);
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& name = ast::Function::NameOf(node);
  auto& function = factory().NewFunction(node);

  const auto* const class_tag =
      maybe_document ? ProcessClassTag(*maybe_document) : nullptr;

  const auto& type_parameters = maybe_document
                                    ? ProcessTemplateTag(*maybe_document)
                                    : std::vector<const TypeParameter*>();

  if (class_tag) {
    auto& class_value =
        factory().NewClass(node, ClassKindOf(*class_tag), type_parameters);
    context().RegisterValue(node, &class_value);
    if (name.Is<ast::Name>())
      BindType(name, type_factory().NewClassType(&class_value));

  } else {
    context().RegisterValue(node, &function);
  }

  if (name == ast::SyntaxCode::Name) {
    auto& variable = BindVariable(VariableKind::Function, name);
    if (!variable.assignments().empty()) {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *variable.assignments().front());
    }
    Value::Editor().AddAssignment(&variable, node);
  }

  Environment environment(this);
  if (!maybe_document)
    return VisitChildNodes(node);
  for (const auto& type_parameter : type_parameters)
    BindType(type_parameter->name(), *type_parameter);
  VisitChildNodes(node);
  Visit(*maybe_document);
}

std::vector<const TypeParameter*> NameResolver::ProcessTemplateTag(
    const ast::Node& document) {
  std::vector<const TypeParameter*> type_parameters;
  for (const auto& node : ast::NodeTraversal::ChildNodesOf(document)) {
    if (!node.Is<ast::JsDocTag>())
      continue;
    if (ast::JsDocTag::NameOf(node) != ast::TokenKind::AtTemplate)
      continue;
    for (const auto& type_name : ast::JsDocTag::OperandsOf(node)) {
      if (!type_name.Is<ast::TypeName>()) {
        AddError(type_name, ErrorCode::ENVIRONMENT_EXPECT_NAME);
        continue;
      }
      const auto& name = ast::TypeName::NameOf(type_name);
      const auto& type = type_factory().NewTypeParameter(name);
      type_parameters.push_back(&type.As<TypeParameter>());
    }
  }
  return type_parameters;
}

void NameResolver::ProcessVariableDeclaration(VariableKind kind,
                                              const ast::Node& node,
                                              const ast::Node& document) {
  DCHECK(node.syntax().Is<ast::VariableDeclaration>()) << node;
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  const auto* kind_tag = ClassifyDocument(document);
  if (!kind_tag) {
    ProcessDocument(document);
    Visit(node);
    return;
  }
  const auto& binding = node.child_at(0);
  if (node.arity() != 1 || !binding.Is<ast::BindingNameElement>()) {
    AddError(node, ErrorCode::ENVIRONMENT_INVALID_CONSTRUCTOR);
    Visit(document);
    Visit(node);
    return;
  }
  const auto& name = ast::BindingNameElement::NameOf(binding);
  auto& variable = BindVariable(kind, name);
  context().RegisterValue(binding, &variable);
  Value::Editor().AddAssignment(&variable, binding);
  if (variable.assignments().size() > 1) {
    AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             *variable.assignments().front());
    return;
  }
  const auto& initializer = ast::BindingNameElement::InitializerOf(binding);
  if (ast::JsDocTag::NameOf(*kind_tag) == ast::TokenKind::AtTypeDef) {
    ProcessDocument(document);
    if (!initializer.Is<ast::ElisionExpression>())
      AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_INITIALIZER);
    BindType(name, type_factory().NewTypeAlias(name, kind_tag->child_at(1)));
    return;
  }
  const auto& class_tag = *kind_tag;
  DCHECK(IsClassTag(class_tag));
  const auto& type_parameters = ProcessTemplateTag(document);
  if (initializer.Is<ast::ElisionExpression>()) {
    auto& class_value =
        factory().NewClass(node, ClassKindOf(class_tag), type_parameters);
    context().RegisterValue(initializer, &class_value);
    ProcessDocument(document);
    BindType(name, type_factory().NewClassType(&class_value));
    return;
  }
  if (initializer.Is<ast::Class>()) {
    ProcessClass(initializer, &document);
  }
  if (initializer.Is<ast::Function>()) {
    ProcessFunction(initializer, &document);
  } else {
    Visit(initializer);
    ProcessDocument(document);
  }
  auto* const value = context().TryValueOf(initializer);
  if (!value)
    return;
  auto* const class_value = value->TryAs<Class>();
  if (!class_value)
    return;
  BindType(name, type_factory().NewClassType(class_value));
}

// AST node handlers
const Class* NameResolver::TryClassOfPrototype(const ast::Node& node) const {
  if (!node.Is<ast::MemberExpression>())
    return nullptr;
  if (ast::MemberExpression::NameOf(node) != ast::TokenKind::Prototype)
    return nullptr;
  const auto& container = ast::MemberExpression::ContainerOf(node);
  const auto* const holder = context().TryValueOf(container);
  if (!holder || !holder->Is<ValueHolder>())
    return nullptr;
  if (holder->As<ValueHolder>().assignments().size() != 1)
    return nullptr;
  const auto* const value =
      context().TryValueOf(*holder->As<ValueHolder>().assignments().front());
  if (!value || !value->Is<Class>())
    return nullptr;
  return &value->As<Class>();
}

void NameResolver::VisitChildNodes(const ast::Node& node) {
  ancestors_.push_back(&node);
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(node))
    Visit(child);
  ancestors_.pop_back();
}

//
// ast::NodeVisitor members
//

void NameResolver::VisitDefault(const ast::Node& node) {
  VisitChildNodes(node);
}

// Binding elements
void NameResolver::VisitInternal(const ast::BindingNameElement& syntax,
                                 const ast::Node& node) {
  DCHECK(variable_kind_ == VariableKind::Const ||
         variable_kind_ == VariableKind::Let ||
         variable_kind_ == VariableKind::Parameter ||
         variable_kind_ == VariableKind::Var)
      << variable_kind_;
  VisitDefault(node);
  auto& variable =
      BindVariable(variable_kind_, ast::BindingNameElement::NameOf(node));
  Value::Editor().AddAssignment(&variable, node);
  context().RegisterValue(node, &variable);
  if (variable.assignments().size() == 1)
    return;
  AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
           *variable.assignments().front());
}

// Declarations
void NameResolver::VisitInternal(const ast::Annotation& syntax,
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
  const auto& document = ast::Annotation::DocumentOf(node);
  if (annotated.Is<ast::Class>())
    return ProcessClass(annotated, &document);
  if (annotated.Is<ast::ConstStatement>())
    return ProcessVariableDeclaration(VariableKind::Const, annotated, document);
  if (annotated.Is<ast::Function>())
    return ProcessFunction(annotated, &document);
  if (annotated.Is<ast::LetStatement>())
    return ProcessVariableDeclaration(VariableKind::Let, annotated, document);
  if (annotated.Is<ast::VarStatement>())
    return ProcessVariableDeclaration(VariableKind::Var, annotated, document);
  if (annotated.Is<ast::GroupExpression>())
    return VisitChildNodes(node);
  if (!annotated.Is<ast::ExpressionStatement>()) {
    AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
    return;
  }
  const auto& expression = ast::ExpressionStatement::ExpressionOf(annotated);
  if (expression.Is<ast::AssignmentExpression>()) {
    const auto& lhs = ast::AssignmentExpression::LeftHandSideOf(expression);
    const auto& rhs = ast::AssignmentExpression::RightHandSideOf(expression);
    Visit(lhs);
    if (!IsMemberExpression(lhs)) {
      AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
      Visit(rhs);
      return;
    }
    Environment environment(this);
    if (const auto* class_value = TryClassOfPrototype(lhs.child_at(0)))
      BindTypeParameters(*class_value);
    if (rhs.Is<ast::Class>()) {
      ProcessClass(rhs, &document);
      return;
    }
    if (rhs.Is<ast::Function>()) {
      ProcessFunction(rhs, &document);
      return;
    }
    ProcessDocument(document);
    Visit(annotated);
    return;
  }

  if (IsMemberExpression(expression)) {
    Visit(expression);
    const auto& member = expression.child_at(0);
    Environment environment(this);
    if (const auto* class_value = TryClassOfPrototype(member))
      BindTypeParameters(*class_value);
    if (const auto* class_tag = ProcessClassTag(document)) {
      const auto& type_parameters = ProcessTemplateTag(document);
      const auto& class_kind = ClassKindOf(*class_tag);
      auto& class_value =
          factory().NewClass(expression, class_kind, type_parameters);
      auto& property = context().ValueOf(expression).As<Property>();
      Value::Editor().AddAssignment(&property, document);
      context().RegisterValue(document, &class_value);
    }
    ProcessDocument(document);
    return;
  }

  VisitDefault(node);
  AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
}

void NameResolver::VisitInternal(const ast::Class& syntax,
                                 const ast::Node& node) {
  ProcessClass(node, nullptr);
}

void NameResolver::VisitInternal(const ast::Function& syntax,
                                 const ast::Node& node) {
  ProcessFunction(node, nullptr);
}

void NameResolver::VisitInternal(const ast::Method& syntax,
                                 const ast::Node& node) {
  NOTREACHED() << "Method should be processed in Class " << node;
}

// Expressions
void NameResolver::VisitInternal(const ast::AssignmentExpression& syntax,
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

void NameResolver::VisitInternal(const ast::ComputedMemberExpression& syntax,
                                 const ast::Node& node) {
  VisitDefault(node);
  auto* const value =
      context().TryValueOf(ast::ComputedMemberExpression::ExpressionOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& key = ast::ComputedMemberExpression::ExpressionOf(node);
  if (!ast::IsKnownSymbol(key))
    return;
  auto& property = factory().GetOrNewProperty(&properties, key);
  context().RegisterValue(node, &property);
}

void NameResolver::VisitInternal(const ast::MemberExpression& syntax,
                                 const ast::Node& node) {
  VisitDefault(node);
  auto* const value =
      context().TryValueOf(ast::MemberExpression::ContainerOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& name = ast::MemberExpression::NameOf(node);
  auto& property = factory().GetOrNewProperty(&properties, name);
  context().RegisterValue(node, &property);
}

void NameResolver::VisitInternal(const ast::ParameterList& syntax,
                                 const ast::Node& node) {
  base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Parameter);
  VisitDefault(node);
}

void NameResolver::VisitInternal(const ast::ReferenceExpression& syntax,
                                 const ast::Node& node) {
  const auto& name = ast::ReferenceExpression::NameOf(node);
  if (ast::Name::IsKeyword(name))
    return;
  if (auto* present = FindVariable(name)) {
    context().RegisterValue(node, present);
    return;
  }
  environment_->AddForwardReferencedVariable(node);
}

// Statements
void NameResolver::VisitInternal(const ast::BlockStatement& syntax,
                                 const ast::Node& node) {
  Environment environment(this);
  VisitDefault(node);
}

void NameResolver::VisitInternal(const ast::ConstStatement& syntax,
                                 const ast::Node& node) {
  base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Const);
  VisitDefault(node);
}

void NameResolver::VisitInternal(const ast::LetStatement& syntax,
                                 const ast::Node& node) {
  base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Let);
  VisitDefault(node);
}

void NameResolver::VisitInternal(const ast::VarStatement& syntax,
                                 const ast::Node& node) {
  base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Var);
  VisitDefault(node);
}

// Types
void NameResolver::VisitInternal(const ast::TypeName& syntax,
                                 const ast::Node& node) {
  if (const auto* present = FindType(ast::TypeName::NameOf(node))) {
    context().RegisterType(node, *present);
    return;
  }
  environment_->AddForwardReferencedType(node);
}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <utility>

#include "joana/analyzer/name_resolver.h"

#include "base/auto_reset.h"
#include "joana/analyzer/annotation_compiler.h"
#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/properties_editor.h"
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

VariableKind VariableKindOf(const ast::Node& keyword) {
  if (keyword == ast::TokenKind::Const)
    return VariableKind::Const;
  if (keyword == ast::TokenKind::Let)
    return VariableKind::Let;
  if (keyword == ast::TokenKind::Var)
    return VariableKind::Var;
  if (keyword == ast::TokenKind::Var)
    return VariableKind::Var;
  if (keyword.Is<ast::Empty>())
    return VariableKind::Function;
  NOTREACHED() << keyword;
  return VariableKind::Function;
}

}  // namespace

//
// NameResolver::Environment
//
class NameResolver::Environment final {
 public:
  explicit Environment(NameResolver* resolver);
  ~Environment();

  bool is_global() const { return !outer_; }
  const Environment* outer() const { return outer_; }
  Environment* outer() { return outer_; }

  void AddForwardReferencedType(const ast::Node& node);
  void AddForwardReferencedVariable(const ast::Node& node);

  void BindType(const ast::Node& name, const Type& type);
  void BindVariable(const ast::Node& name, const Variable& value);
  std::pair<const ast::Node*, const Type*> FindNameAndType(
      const ast::Node& name) const;
  const Type* FindType(const ast::Node& name) const;
  const Variable* FindVariable(const ast::Node& name) const;

  static std::unique_ptr<Environment> NewGlobalEnvironment(
      NameResolver* resolver);

 private:
  void ResolveForwardReferences();

  std::vector<const ast::Node*> forward_referenced_types_;
  std::vector<const ast::Node*> forward_referenced_variables_;
  Environment* const outer_;
  NameResolver& resolver_;
  std::unordered_map<int, std::pair<const ast::Node*, const Type*>> type_map_;
  std::unordered_map<int, const Variable*> value_map_;

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
                                             const Variable& value) {
  const auto& result = value_map_.emplace(ast::Name::IdOf(name), &value);
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

const Variable* NameResolver::Environment::FindVariable(
    const ast::Node& name) const {
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
    auto& data = resolver->factory().NewValueHolderData();
    auto& properties = resolver->context().global_properties();
    const auto& variable = resolver->factory().NewVariable(
        VariableKind::Const, name, &data, &properties);
    Value::Editor().AddAssignment(variable, name);
    const auto& property = resolver->factory().NewProperty(
        Visibility::Public, name, &data, &properties);
    Properties::Editor().Add(&properties, property);
    environment->BindVariable(name, variable);
  }
  return std::move(environment);
}

void NameResolver::Environment::ResolveForwardReferences() {
  for (const auto& node : ReferenceRangeOf(forward_referenced_variables_)) {
    const auto* const present =
        FindVariable(ast::ReferenceExpression::NameOf(node));
    if (present) {
      resolver_.context().RegisterValue(node, *present);
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
  factory().ResetCurrentId();
  type_factory().ResetCurrentId();
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
  if (!class_value.Is<GenericClass>())
    return;
  for (const auto& parameter : class_value.As<GenericClass>().parameters())
    BindType(parameter.name(), parameter);
}

const Type* NameResolver::FindType(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindType(name))
      return present;
  }
  return nullptr;
}

const Variable& NameResolver::BindVariable(VariableKind kind,
                                           const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  if (auto* present = environment_->FindVariable(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             present->node());
    return *present;
  }
  auto& data = factory().NewValueHolderData();
  auto& properties = factory().NewProperties(name);
  const auto& variable = factory().NewVariable(kind, name, &data, &properties);
  environment_->BindVariable(name, variable);
  // TODO(eval1749): Expose global "var" binding to global object.
  if (!environment_->is_global())
    return variable;
  const auto& property =
      factory().NewProperty(Visibility::Public, name, &data, &properties);
  Properties::Editor().Add(&context().global_properties(), property);
  return variable;
}

const Variable* NameResolver::FindVariable(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->FindVariable(name))
      return present;
  }
  return nullptr;
}

const Property& NameResolver::GetOrNewProperty(Properties* properties,
                                               const ast::Node& node) {
  if (auto* present = properties->TryGet(node))
    return *present;
  const auto& property = NewProperty(Visibility::Public, node);
  Properties::Editor().Add(properties, property);
  return property;
}

const Class& NameResolver::NewClass(
    ClassKind kind,
    const ast::Node& name,
    const ast::Node& node,
    const std::vector<const TypeParameter*>& parameters,
    Properties* passed_properties) {
  auto& properties =
      passed_properties ? *passed_properties : factory().NewProperties(node);
  const auto& class_value =
      parameters.empty()
          ? factory().NewNormalClass(kind, name, node, &properties)
          : factory().NewGenericClass(kind, name, node, parameters,
                                      &properties);
  const auto& prototype_name =
      BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Prototype);
  if (properties.TryGet(prototype_name))
    return class_value;
  const auto& prototype_property =
      NewProperty(Visibility::Public, prototype_name);
  Value::Editor().AddAssignment(prototype_property, node);
  Properties::Editor().Add(&class_value.properties(), prototype_property);
  return class_value;
}

const Property& NameResolver::NewProperty(Visibility visibility,
                                          const ast::Node& node) {
  return factory().NewProperty(visibility, node,
                               &factory().NewValueHolderData(),
                               &factory().NewProperties(node));
}

void NameResolver::ProcessAssignment(const ast::Node& lhs,
                                     const ast::Node* maybe_rhs,
                                     const ast::Node& name,
                                     const Annotation& annotation) {
  if (maybe_rhs) {
    const auto& rhs = *maybe_rhs;
    if (rhs.Is<ast::Class>())
      return ProcessClass(rhs, annotation, &name);
    if (rhs.Is<ast::Function>())
      return ProcessFunction(rhs, annotation, &name);
    Visit(rhs);
  }
  ProcessTemplateTags(annotation);
  ProcessParamTags(annotation);
  switch (annotation.kind()) {
    case Annotation::Kind::Constructor:
    case Annotation::Kind::Interface: {
      const auto& type_parameters =
          ProcessTypeParameterNames(annotation.type_parameter_names());
      auto& class_value =
          NewClass(annotation.class_kind(), name, lhs, type_parameters);
      if (maybe_rhs && !maybe_rhs->Is<ast::ElisionExpression>())
        AddError(lhs, ErrorCode::ENVIRONMENT_INVALID_CONSTRUCTOR);
      if (!maybe_rhs)
        context().RegisterValue(annotation.document(), class_value);
      else if (maybe_rhs->Is<ast::ElisionExpression>())
        context().RegisterValue(*maybe_rhs, class_value);
      const auto& class_type = type_factory().NewClassType(class_value);
      if (!lhs.Is<ast::BindingNameElement>())
        return;
      BindType(name, class_type);
      return;
    }
    case Annotation::Kind::Define:
    case Annotation::Kind::Dict:
    case Annotation::Kind::None:
    case Annotation::Kind::Type:
      return;
    case Annotation::Kind::Enum:
      DVLOG(0) << "NYI @enum";
      return;
    case Annotation::Kind::Function: {
      if (!maybe_rhs) {
        const auto& function =
            factory().NewFunction(name, annotation.document());
        context().RegisterValue(annotation.document(), function);
        return;
      }
      if (maybe_rhs->Is<ast::ElisionExpression>()) {
        auto& function = factory().NewFunction(name, annotation.document());
        context().RegisterValue(*maybe_rhs, function);
        return;
      }
      return;
    }
    case Annotation::Kind::TypeDef: {
      if (maybe_rhs && !maybe_rhs->Is<ast::ElisionExpression>())
        AddError(lhs, ErrorCode::ENVIRONMENT_UNEXPECT_INITIALIZER);
      const auto& type =
          type_factory().NewTypeAlias(name, annotation.kind_tag()->child_at(1));
      if (!lhs.Is<ast::BindingNameElement>())
        return;
      BindType(name, type);
      return;
    }
  }
  NOTREACHED() << "We should handle annotation " << *annotation.kind_tag();
}

void NameResolver::ProcessClass(const ast::Node& node,
                                const Annotation& annotation,
                                const ast::Node* maybe_alias) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  // TODO(eval1749): We should check annotation for class to check
  // @interface and @record.
  const auto& type_parameters =
      ProcessTypeParameterNames(annotation.type_parameter_names());

  const auto& real_name = ast::Class::NameOf(node);
  const auto& class_name =
      real_name.Is<ast::Empty>() && maybe_alias ? *maybe_alias : real_name;
  const auto* const variable =
      real_name.Is<ast::Name>() ? &BindVariable(VariableKind::Class, real_name)
                                : nullptr;
  auto& class_properties =
      variable ? variable->properties() : factory().NewProperties(node);

  const auto& class_value = NewClass(annotation.class_kind(), class_name, node,
                                     type_parameters, &class_properties);
  context().RegisterValue(node, class_value);
  if (maybe_alias)
    BindType(*maybe_alias, type_factory().NewClassType(class_value));
  if (variable) {
    if (variable->assignments().empty()) {
      BindType(real_name, type_factory().NewClassType(class_value));
    } else {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *variable->assignments().front());
    }
    Value::Editor().AddAssignment(*variable, node);
  }

  // Class wide template parameters.
  Environment environment(this);
  BindTypeParameters(class_value);
  if (annotation.has_document())
    Visit(annotation.document());

  Visit(ast::Class::HeritageOf(node));

  // Populate class properties and prototype properties with methods.
  for (const auto& child :
       ast::NodeTraversal::ChildNodesOf(ast::Class::BodyOf(node))) {
    if (child.Is<ast::Method>()) {
      ProcessMethod(child, Annotation(), class_value);
      continue;
    }
    if (child.Is<ast::Annotation>()) {
      const auto& member = ast::Annotation::AnnotatedOf(child);
      if (member != ast::SyntaxCode::Method) {
        AddError(member, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
        continue;
      }
      const auto& method_annotation =
          Annotation::Compiler(&context())
              .Compile(ast::Annotation::DocumentOf(child), member);
      ProcessMethod(member, method_annotation, class_value);
      continue;
    }
    AddError(child, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
  }
}

void NameResolver::ProcessMethod(const ast::Node& method_node,
                                 const Annotation& annotation,
                                 const Class& class_value) {
  const auto& method_name = ast::Method::NameOf(method_node);
  if (method_name == ast::TokenKind::Constructor &&
      !IsValidConstructor(method_node)) {
    AddError(method_node, ErrorCode::ENVIRONMENT_INVALID_CONSTRUCTOR);
  }

  // Check multiple occurrence
  Properties& properties = ast::Method::IsStatic(method_node)
                               ? class_value.properties()
                               : class_value.prototype_properties();
  const auto& method = factory().NewFunction(method_name, method_node);
  context().RegisterValue(method_node, method);
  if (auto* present = properties.TryGet(method_name)) {
    AddError(method_node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
             present->node());
    Value::Editor().AddAssignment(*present, method_node);
  } else {
    const auto& property = NewProperty(Visibility::Public, method_name);
    Properties::Editor().Add(&properties, property);
    Value::Editor().AddAssignment(property, method_node);
  }
  Environment environment(this);
  BindTypeParameters(class_value);
  const auto& type_parameters =
      ProcessTypeParameterNames(annotation.type_parameter_names());
  for (const auto& type_parameter : type_parameters)
    BindType(type_parameter->name(), *type_parameter);
  if (annotation.has_document())
    Visit(annotation.document());
  VisitChildNodes(method_node);
}

// Bind name in "@param {type} name" tags.
void NameResolver::ProcessParamTags(const Annotation& annotation) {
  Environment environment(this);
  for (const auto& parameter_tag : annotation.parameter_tags()) {
    const auto& reference = parameter_tag->child_at(2);
    if (!reference.Is<ast::ReferenceExpression>())
      continue;
    BindVariable(VariableKind::Parameter,
                 ast::ReferenceExpression::NameOf(reference));
  }
  Visit(annotation.document());
}

void NameResolver::ProcessFunction(const ast::Node& node,
                                   const Annotation& annotation,
                                   const ast::Node* maybe_alias) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& real_name = ast::Function::NameOf(node);
  const auto& type_parameters =
      ProcessTypeParameterNames(annotation.type_parameter_names());
  const auto& function_name =
      real_name.Is<ast::Empty>() && maybe_alias ? *maybe_alias : real_name;
  const auto& function = factory().NewFunction(function_name, node);
  const auto* const variable =
      real_name.Is<ast::Name>()
          ? &BindVariable(VariableKind::Function, real_name)
          : nullptr;

  if (annotation.is_constructor() || annotation.is_interface()) {
    auto& class_value =
        NewClass(annotation.class_kind(), function.name(), node,
                 type_parameters, variable ? &variable->properties() : nullptr);
    context().RegisterValue(node, class_value);
    if (maybe_alias)
      BindType(*maybe_alias, type_factory().NewClassType(class_value));
  } else {
    context().RegisterValue(node, function);
  }

  if (variable) {
    if (variable->assignments().empty()) {
      auto* class_value = context().TryValueOf(node);
      if (class_value && class_value->Is<Class>())
        BindType(real_name,
                 type_factory().NewClassType(class_value->As<Class>()));
    } else {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *variable->assignments().front());
    }
    Value::Editor().AddAssignment(*variable, node);
  }
  Environment environment(this);
  for (const auto& type_parameter : type_parameters)
    BindType(type_parameter->name(), *type_parameter);
  if (annotation.has_document())
    Visit(annotation.document());
  VisitChildNodes(node);
}

void NameResolver::ProcessPropertyAssignment(const ast::Node& lhs,
                                             const ast::Node* maybe_rhs,
                                             const Annotation& annotation) {
  DCHECK(IsMemberExpression(lhs)) << lhs;
  const auto& container = lhs.child_at(0);
  const auto& key = lhs.child_at(1);
  Visit(container);
  auto* const value = context().TryValueOf(container);
  if (!value || !value->Is<Object>()) {
    ProcessTemplateTags(annotation);
    ProcessParamTags(annotation);
    if (maybe_rhs)
      Visit(*maybe_rhs);
    return;
  }
  if (key.Is<ast::Name>()) {
    auto& properties = value->As<Object>().properties();
    if (auto* present = properties.TryGet(key)) {
      AddError(lhs, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               present->node());
    } else {
      const auto& property = NewProperty(annotation.visibility(), key);
      Properties::Editor().Add(&properties, property);
      context().RegisterValue(lhs, property);
      Value::Editor().AddAssignment(
          property, maybe_rhs ? *maybe_rhs : annotation.document());
    }
  } else if (ast::IsKnownSymbol(key)) {
    Visit(key);
    auto& properties = value->As<Object>().properties();
    if (auto* present = properties.TryGet(key)) {
      AddError(lhs, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               present->node());
    } else {
      const auto& property = NewProperty(annotation.visibility(), key);
      Properties::Editor().Add(&properties, property);
      context().RegisterValue(lhs, property);
      Value::Editor().AddAssignment(
          property, maybe_rhs ? *maybe_rhs : annotation.document());
    }
  } else {
    AddError(lhs, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
    Visit(key);
  }
  Environment environment(this);
  if (const auto* class_value = TryClassOfPrototype(container))
    BindTypeParameters(*class_value);
  ProcessAssignment(lhs, maybe_rhs, key, annotation);
}

// Bind name of @template tags
void NameResolver::ProcessTemplateTags(const Annotation& annotation) {
  for (const auto& type_parameter :
       ProcessTypeParameterNames(annotation.type_parameter_names())) {
    BindType(type_parameter->name(), *type_parameter);
  }
}

std::vector<const TypeParameter*> NameResolver::ProcessTypeParameterNames(
    const std::vector<const ast::Node*>& type_names) {
  std::vector<const TypeParameter*> type_parameters;
  for (const auto& type_name : type_names) {
    const auto& name = ast::TypeName::NameOf(*type_name);
    const auto& type = type_factory().NewTypeParameter(name);
    type_parameters.push_back(&type.As<TypeParameter>());
  }
  return type_parameters;
}

void NameResolver::ProcessVariableDeclaration(VariableKind variable_kind,
                                              const ast::Node& node,
                                              const Annotation& annotation) {
  DCHECK(node.syntax().Is<ast::VariableDeclaration>()) << node;
  base::AutoReset<VariableKind> scope(&variable_kind_, variable_kind);
  for (const auto& binding : ast::NodeTraversal::ChildNodesOf(node)) {
    if (annotation.is_type() || annotation.is_none()) {
      ProcessTemplateTags(annotation);
      ProcessParamTags(annotation);
      Visit(binding);
      continue;
    }
    if (!binding.Is<ast::BindingNameElement>()) {
      if (!annotation.is_type() && !annotation.is_none()) {
        AddError(*annotation.kind_tag(),
                 ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION, binding);
      }
      ProcessTemplateTags(annotation);
      ProcessParamTags(annotation);
      Visit(binding);
      continue;
    }
    const auto& name = ast::BindingNameElement::NameOf(binding);
    const auto& variable = BindVariable(variable_kind, name);
    context().RegisterValue(binding, variable);
    Value::Editor().AddAssignment(variable, binding);
    if (variable.assignments().size() > 1) {
      AddError(node, ErrorCode::ENVIRONMENT_MULTIPLE_OCCURRENCES,
               *variable.assignments().front());
      return;
    }
    ProcessAssignment(binding, &ast::BindingNameElement::InitializerOf(binding),
                      name, annotation);
  }
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
  DCHECK(variable_kind_ == VariableKind::Catch ||
         variable_kind_ == VariableKind::Const ||
         variable_kind_ == VariableKind::Let ||
         variable_kind_ == VariableKind::Parameter ||
         variable_kind_ == VariableKind::Var)
      << variable_kind_;
  const auto& name = ast::BindingNameElement::NameOf(node);
  const auto& initializer = ast::BindingNameElement::InitializerOf(node);
  if (initializer.Is<ast::Class>())
    ProcessClass(initializer, Annotation(), &name);
  else if (initializer.Is<ast::Function>())
    ProcessFunction(initializer, Annotation(), &name);
  else
    Visit(initializer);
  const auto& variable = BindVariable(variable_kind_, name);
  Value::Editor().AddAssignment(variable, node);
  context().RegisterValue(node, variable);
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
  const auto& annotation =
      Annotation::Compiler(&context()).Compile(document, annotated);
  if (annotated.Is<ast::Class>())
    return ProcessClass(annotated, annotation, nullptr);
  if (annotated.Is<ast::ConstStatement>()) {
    ProcessVariableDeclaration(VariableKind::Const, annotated, annotation);
    return;
  }
  if (annotated.Is<ast::Function>())
    return ProcessFunction(annotated, annotation, nullptr);
  if (annotated.Is<ast::LetStatement>())
    return ProcessVariableDeclaration(VariableKind::Let, annotated, annotation);
  if (annotated.Is<ast::VarStatement>())
    return ProcessVariableDeclaration(VariableKind::Var, annotated, annotation);
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
    if (!IsMemberExpression(lhs)) {
      AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
      VisitChildNodes(node);
      return;
    }
    ProcessPropertyAssignment(lhs, &rhs, annotation);
    return;
  }

  if (IsMemberExpression(expression))
    return ProcessPropertyAssignment(expression, nullptr, annotation);

  VisitDefault(node);
  AddError(node, ErrorCode::ENVIRONMENT_UNEXPECT_ANNOTATION);
}

void NameResolver::VisitInternal(const ast::Class& syntax,
                                 const ast::Node& node) {
  ProcessClass(node, Annotation(), nullptr);
}

void NameResolver::VisitInternal(const ast::Function& syntax,
                                 const ast::Node& node) {
  ProcessFunction(node, Annotation(), nullptr);
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
  const auto& holder = value->As<ValueHolder>();
  Value::Editor().AddAssignment(holder, node);
}

void NameResolver::VisitInternal(const ast::ComputedMemberExpression& syntax,
                                 const ast::Node& node) {
  VisitDefault(node);
  auto* const value =
      context().TryValueOf(ast::ComputedMemberExpression::ContainerOf(node));
  if (!value || !value->Is<Object>())
    return;
  auto& properties = value->As<Object>().properties();
  const auto& key = ast::ComputedMemberExpression::ExpressionOf(node);
  if (!ast::IsKnownSymbol(key))
    return;
  const auto& property = GetOrNewProperty(&properties, key);
  context().RegisterValue(node, property);
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
  const auto& property = GetOrNewProperty(&properties, name);
  context().RegisterValue(node, property);
}

void NameResolver::VisitInternal(const ast::ObjectInitializer& syntax,
                                 const ast::Node& node) {
  const auto& empty_name =
      BuiltInWorld::GetInstance()->NameOf(ast::TokenKind::Object);
  auto& properties = factory().NewProperties(empty_name);
  const auto& class_value =
      NewClass(ClassKind::Class, empty_name, node, {}, &properties);
  context().RegisterValue(node, class_value);
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(node)) {
    // TODO(eval1749): We should report error for multiple occurrences of
    // property names.
    if (child.Is<ast::Annotation>()) {
      const auto& member = ast::Annotation::AnnotatedOf(child);
      if (member != ast::SyntaxCode::Method) {
        AddError(member, ErrorCode::ENVIRONMENT_EXPECT_METHOD);
        continue;
      }
      const auto& method_annotation =
          Annotation::Compiler(&context())
              .Compile(ast::Annotation::DocumentOf(child), member);
      ProcessMethod(member, method_annotation, class_value);
      // TODO(eval1749): We should get visibility from annotation.
      GetOrNewProperty(&properties, ast::Method::NameOf(member));
      continue;
    }
    if (child.Is<ast::DelimiterExpression>())
      continue;
    if (child.Is<ast::Method>()) {
      GetOrNewProperty(&properties, ast::Method::NameOf(child));
      ProcessMethod(child, Annotation(), class_value);
      continue;
    }
    if (child.Is<ast::ReferenceExpression>()) {
      GetOrNewProperty(&properties, ast::ReferenceExpression::NameOf(child));
      Visit(child);
      continue;
    }
    if (child.Is<ast::Property>()) {
      const auto& property_name = ast::Property::NameOf(child);
      if (property_name.Is<ast::Name>()) {
        const auto& property = GetOrNewProperty(&properties, property_name);
        context().RegisterValue(child, property);
      }
      Visit(ast::Property::ValueOf(child));
      continue;
    }
    AddError(child, ErrorCode::ENVIRONMENT_EXPECT_OBJECT_MEMBER);
  }
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
    context().RegisterValue(node, *present);
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

void NameResolver::VisitInternal(const ast::CatchClause& syntax,
                                 const ast::Node& node) {
  Environment environment(this);
  {
    base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Catch);
    Visit(ast::CatchClause::ParameterOf(node));
  }
  Visit(ast::CatchClause::StatementOf(node));
}

void NameResolver::VisitInternal(const ast::ConstStatement& syntax,
                                 const ast::Node& node) {
  base::AutoReset<VariableKind> scope(&variable_kind_, VariableKind::Const);
  VisitDefault(node);
}

void NameResolver::VisitInternal(const ast::ForInStatement& syntax,
                                 const ast::Node& node) {
  const auto variable_kind =
      VariableKindOf(ast::ForInStatement::KeywordOf(node));
  if (variable_kind == VariableKind::Function) {
    Visit(ast::ForInStatement::BindingOf(node));
    Visit(ast::ForInStatement::ExpressionOf(node));
    Visit(ast::ForInStatement::StatementOf(node));
    return;
  }
  Environment environment(this);
  {
    base::AutoReset<VariableKind> scope(&variable_kind_, variable_kind);
    Visit(ast::ForInStatement::BindingOf(node));
  }
  Visit(ast::ForInStatement::ExpressionOf(node));
  Visit(ast::ForInStatement::StatementOf(node));
}

void NameResolver::VisitInternal(const ast::ForOfStatement& syntax,
                                 const ast::Node& node) {
  const auto variable_kind =
      VariableKindOf(ast::ForOfStatement::KeywordOf(node));
  if (variable_kind == VariableKind::Function) {
    Visit(ast::ForOfStatement::BindingOf(node));
    Visit(ast::ForOfStatement::ExpressionOf(node));
    Visit(ast::ForOfStatement::StatementOf(node));
    return;
  }
  Environment environment(this);
  {
    base::AutoReset<VariableKind> scope(&variable_kind_, variable_kind);
    Visit(ast::ForOfStatement::BindingOf(node));
  }
  Visit(ast::ForOfStatement::ExpressionOf(node));
  Visit(ast::ForOfStatement::StatementOf(node));
}

void NameResolver::VisitInternal(const ast::ForStatement& syntax,
                                 const ast::Node& node) {
  const auto variable_kind = VariableKindOf(ast::ForStatement::KeywordOf(node));
  if (variable_kind == VariableKind::Function) {
    Visit(ast::ForStatement::InitializeOf(node));
    Visit(ast::ForStatement::ConditionOf(node));
    Visit(ast::ForStatement::StepOf(node));
    Visit(ast::ForStatement::StatementOf(node));
    return;
  }
  Environment environment(this);
  {
    base::AutoReset<VariableKind> scope(&variable_kind_, variable_kind);
    Visit(ast::ForStatement::InitializeOf(node));
  }
  Visit(ast::ForStatement::ConditionOf(node));
  Visit(ast::ForStatement::StepOf(node));
  Visit(ast::ForStatement::StatementOf(node));
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
    if (context().TryTypeOf(node))
      return;
    context().RegisterType(node, *present);
    return;
  }
  environment_->AddForwardReferencedType(node);
}

}  // namespace analyzer
}  // namespace joana

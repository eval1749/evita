// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "joana/analyzer/type_annotation_transformer.h"

#include "base/logging.h"
#include "joana/analyzer/annotation.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/type_name_resolver.h"
#include "joana/analyzer/type_transformer.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node& AnnotatedNodeOf(const ast::Node& node) {
  if (node.Is<ast::AssignmentExpression>())
    return AnnotatedNodeOf(ast::AssignmentExpression::RightHandSideOf(node));
  if (node.Is<ast::ComputedMemberExpression>())
    return node;
  if (node.Is<ast::Function>())
    return node;
  if (node.Is<ast::MemberExpression>())
    return node;
  if (node.Is<ast::Method>())
    return node;
  if (node.Is<ast::ReferenceExpression>())
    return node;
  return node;
}

bool IsPrototypeProperty(const ast::Node& node) {
  if (!node.Is(ast::SyntaxCode::MemberExpression))
    return false;
  return ast::MemberExpression::NameOf(node) == ast::TokenKind::Prototype;
}

bool IsMethod(const ast::Node& node) {
  if (node.Is(ast::SyntaxCode::AssignmentExpression)) {
    if (!ast::AssignmentExpression::RightHandSideOf(node).Is(
            ast::SyntaxCode::Function))
      return false;
    return IsMethod(ast::AssignmentExpression::LeftHandSideOf(node));
  }
  if (node.Is(ast::SyntaxCode::ComputedMemberExpression)) {
    return IsPrototypeProperty(
        ast::ComputedMemberExpression::ExpressionOf(node));
  }
  if (node.Is(ast::SyntaxCode::MemberExpression))
    return IsPrototypeProperty(ast::MemberExpression::ContainerOf(node));
  if (node.Is(ast::SyntaxCode::ExpressionStatement))
    return IsMethod(ast::ExpressionStatement::ExpressionOf(node));
  if (node.Is(ast::SyntaxCode::Method))
    return true;
  return false;
}

// Returns member part of computed member expression or member expression.
const ast::Node* ContainerOf(const ast::Node& node) {
  if (node.Is(ast::SyntaxCode::ComputedMemberExpression))
    return &ast::ComputedMemberExpression::ExpressionOf(node);
  if (node.Is(ast::SyntaxCode::MemberExpression))
    return &ast::MemberExpression::ContainerOf(node);
  return nullptr;
}

}  // namespace

//
// TypeAnnotationTransformer::FunctionParameters
//
class TypeAnnotationTransformer::FunctionParameters final {
 public:
  class Builder;

  FunctionParameters(const FunctionTypeArity& arity,
                     const std::vector<const Type*>& types);
  FunctionParameters(FunctionParameters&& other);
  ~FunctionParameters();

  const FunctionTypeArity& arity() const { return arity_; }
  const std::vector<const Type*>& types() const { return types_; }

 private:
  FunctionParameters();

  FunctionTypeArity arity_;
  std::vector<const Type*> types_;
};

TypeAnnotationTransformer::FunctionParameters::FunctionParameters(
    const FunctionTypeArity& arity,
    const std::vector<const Type*>& types)
    : arity_(arity), types_(types) {
  DCHECK_GE(arity.minimum, 0);
  DCHECK_LE(arity.minimum, arity.maximum);
  if (arity.has_rest) {
    DCHECK_EQ(static_cast<size_t>(arity.maximum) + 1, types_.size());
    return;
  }
  DCHECK_EQ(static_cast<size_t>(arity.maximum), types_.size()) << types_;
}

TypeAnnotationTransformer::FunctionParameters::FunctionParameters(
    FunctionParameters&& other)
    : arity_(other.arity_), types_(std::move(other.types_)) {
  other.arity_ = FunctionTypeArity();
}

TypeAnnotationTransformer::FunctionParameters::FunctionParameters() = default;
TypeAnnotationTransformer::FunctionParameters::~FunctionParameters() = default;

//
// TypeAnnotationTransformer::FunctionParameters::Builder
//
class TypeAnnotationTransformer::FunctionParameters::Builder final
    : public ContextUser {
 public:
  Builder(Context* context,
          const ast::Node& parameter_list,
          const std::vector<const ast::Node*>& parameter_tags);
  ~Builder();

  FunctionParameters Build();

 private:
  enum class State {
    Optional,
    Required,
    Rest,
  };

  // Returns @param tag for |name|.
  const ast::Node* FindParameterTag(const ast::Node& name) const;
  void ProcessBindingNameElement(const ast::Node& node);
  void ProcessBindingRestElement(const ast::Node& node);
  // Returns true if |name| is not appeared so far.
  bool RecordName(const ast::Node& name);
  void RecordTag(const ast::Node& tag);
  const Type& TransformType(const ast::Node& node);

  FunctionTypeArity arity_;
  const ast::Node& parameter_list_;
  std::vector<const ast::Node*> parameter_names_;
  const std::vector<const ast::Node*> parameter_tags_;
  std::vector<const Type*> parameter_types_;
  FunctionParameters parameters_;
  State state_ = State::Required;
  std::unordered_set<const ast::Node*> used_tags_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

TypeAnnotationTransformer::FunctionParameters::Builder::Builder(
    Context* context,
    const ast::Node& parameter_list,
    const std::vector<const ast::Node*>& parameter_tags)
    : ContextUser(context),
      parameter_list_(parameter_list),
      parameter_tags_(parameter_tags) {
  DCHECK_EQ(parameter_list, ast::SyntaxCode::ParameterList);
}

TypeAnnotationTransformer::FunctionParameters::Builder::~Builder() = default;

TypeAnnotationTransformer::FunctionParameters
TypeAnnotationTransformer::FunctionParameters::Builder::Build() {
  for (const auto& parameter_node :
       ast::NodeTraversal::ChildNodesOf(parameter_list_)) {
    if (parameter_node.Is<ast::BindingNameElement>()) {
      ProcessBindingNameElement(parameter_node);
      continue;
    }
    if (parameter_node.Is<ast::BindingRestElement>()) {
      ProcessBindingRestElement(parameter_node);
      continue;
    }
    NOTREACHED() << "NYI ProcessParameter " << parameter_node;
  }
  for (const auto& parameter_tag : parameter_tags_) {
    if (used_tags_.count(parameter_tag) == 1)
      continue;
    AddError(*parameter_tag, ErrorCode::JSDOC_UNEXPECT_PARAMETER);
  }
  DCHECK_GE(arity_.minimum, 0);
  DCHECK_LE(arity_.minimum, arity_.maximum);
  if (arity_.has_rest)
    DCHECK_EQ(static_cast<size_t>(arity_.maximum) + 1, parameter_types_.size());
  else
    DCHECK_EQ(static_cast<size_t>(arity_.maximum), parameter_types_.size());
  return FunctionParameters(arity_, parameter_types_);
}

const ast::Node*
TypeAnnotationTransformer::FunctionParameters::Builder::FindParameterTag(
    const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto name_id = ast::Name::KindOf(name);
  const auto& it = std::find_if(
      parameter_tags_.begin(), parameter_tags_.end(),
      [&](const ast::Node* parameter_tag) {
        if (parameter_tag->arity() < 2)
          return false;
        const auto& reference = parameter_tag->child_at(2);
        if (!reference.Is<ast::ReferenceExpression>())
          return false;
        return ast::ReferenceExpression::NameOf(reference) == name_id;
      });
  return it == parameter_tags_.end() ? nullptr : *it;
}

void TypeAnnotationTransformer::FunctionParameters::Builder::
    ProcessBindingNameElement(const ast::Node& node) {
  const ast::Node& name = ast::BindingNameElement::NameOf(node);
  const auto has_initializer = !ast::BindingNameElement::InitializerOf(node)
                                    .Is<ast::ElisionExpression>();
  const auto* const tag = FindParameterTag(name);
  const auto is_optional = tag && tag->child_at(1).Is<ast::OptionalType>();
  const auto is_rest = tag && tag->child_at(1).Is<ast::RestType>();
  const auto& type =
      tag ? TransformType(is_optional || is_rest ? tag->child_at(1).child_at(0)
                                                 : tag->child_at(1))
          : unspecified_type();
  if (RecordName(name) && tag)
    RecordTag(*tag);
  if (has_initializer && (is_optional || is_rest))
    AddError(node, ErrorCode::JSDOC_UNEXPECT_INITIALIZER);
  switch (state_) {
    case State::Required:
      parameter_types_.push_back(&type);
      if (is_rest) {
        arity_.has_rest = true;
        state_ = State::Rest;
        return;
      }
      ++arity_.maximum;
      if (has_initializer || is_optional) {
        state_ = State::Optional;
        arity_.maximum = arity_.minimum + 1;
        return;
      }
      arity_.minimum = arity_.maximum;
      return;
    case State::Optional:
      parameter_types_.push_back(&type);
      if (is_rest) {
        arity_.has_rest = true;
        state_ = State::Rest;
        return;
      }
      ++arity_.maximum;
      return;
    case State::Rest:
      AddError(node, ErrorCode::JSDOC_UNEXPECT_REST);
      return;
  }
  NOTREACHED() << "Unexpected state " << static_cast<int>(state_);
}

void TypeAnnotationTransformer::FunctionParameters::Builder::
    ProcessBindingRestElement(const ast::Node& node) {
  if (state_ == State::Rest)
    AddError(node, ErrorCode::JSDOC_UNEXPECT_REST);
  arity_.has_rest = true;
  const auto& binding = node.child_at(0);
  if (binding.Is<ast::BindingNameElement>()) {
    const auto& name = ast::BindingNameElement::NameOf(binding);
    const auto& maybe_tag = FindParameterTag(name);
    if (!maybe_tag) {
      RecordName(name);
      parameter_types_.push_back(&unspecified_type());
      return;
    }
    const auto& tag = *maybe_tag;
    if (RecordName(name))
      RecordTag(tag);
    const auto& type_node = tag.child_at(1);
    if (!type_node.Is<ast::RestType>()) {
      AddError(tag, ErrorCode::JSDOC_EXPECT_REST);
      parameter_types_.push_back(&unspecified_type());
      return;
    }
    const auto& element_type = TransformType(type_node.child_at(0));
    const auto& array_class = context().factory().NewConstructedClass(
        context().TryClassOf(ast::TokenKind::Array)->As<GenericClass>(),
        {&element_type});
    parameter_types_.push_back(
        &context().type_factory().NewClassType(array_class));
    return;
  }
  NOTREACHED() << "NYI parameter binding" << static_cast<int>(state_);
}

bool TypeAnnotationTransformer::FunctionParameters::Builder::RecordName(
    const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto name_id = ast::Name::KindOf(name);
  const auto& it =
      std::find_if(parameter_names_.begin(), parameter_names_.end(),
                   [&](const ast::Node* present) {
                     return ast::Name::KindOf(*present) == name_id;
                   });
  if (it == parameter_names_.end())
    return true;
  AddError(name, ErrorCode::JSDOC_MULTIPLE_PARAMETER, **it);
  return false;
}

void TypeAnnotationTransformer::FunctionParameters::Builder::RecordTag(
    const ast::Node& tag) {
  DCHECK_EQ(ast::JsDocTag::NameOf(tag), ast::TokenKind::AtParam);
  const auto& result = used_tags_.emplace(&tag);
  DCHECK(result.second) << tag;
}

const Type&
TypeAnnotationTransformer::FunctionParameters::Builder::TransformType(
    const ast::Node& node) {
  if (!node.syntax().Is<ast::Type>()) {
    AddError(node, ErrorCode::JSDOC_EXPECT_TYPE);
    return unspecified_type();
  }
  return TypeTransformer(&context()).Transform(node);
}

//
// TypeAnnotationTransformer
//
TypeAnnotationTransformer::TypeAnnotationTransformer(
    Context* context,
    const Annotation& annotation,
    const ast::Node& node,
    const Type* this_type)
    : ContextUser(context),
      annotation_(annotation),
      node_(node),
      this_type_(this_type) {
  DCHECK(!this_type || this_type->Is<ClassType>()) << this_type;
}

TypeAnnotationTransformer::~TypeAnnotationTransformer() = default;

const Type* TypeAnnotationTransformer::Compile() {
  switch (annotation_.kind()) {
    case Annotation::Kind::Constructor:
    case Annotation::Kind::Function:
      return &TransformAsFunctionType();
    case Annotation::Kind::Enum:
      DVLOG(0) << "NYI enum type" << annotation_.document() << ' ' << node_;
      return nullptr;
    case Annotation::Kind::Interface:
      return &TransformAsInterface();
    case Annotation::Kind::Type:
    case Annotation::Kind::TypeDef: {
      const auto& type_node = annotation_.type_node();
      if (!type_node.syntax().Is<ast::Type>()) {
        AddError(type_node, ErrorCode::JSDOC_EXPECT_TYPE);
        return nullptr;
      }
      return &TransformType(type_node);
    }
  }
  return nullptr;
}

const Type& TypeAnnotationTransformer::ComputeReturnType() {
  if (!annotation_.return_tag())
    return void_type();
  const auto& type_node = annotation_.return_tag()->child_at(1);
  if (type_node.syntax().Is<ast::Type>())
    return TransformType(type_node);
  AddError(type_node, ErrorCode::JSDOC_EXPECT_TYPE);
  return unspecified_type();
}

// Type of "this" is determined by:
// - Class passed by caller during processing class declaration.
// - Class.prototype.Name;
// - Class.prototype[Expression];
const Type& TypeAnnotationTransformer::ComputeThisType() {
  return this_type_ ? *this_type_ : void_type();
}

// Returns type of |Expression| where |Expression| '.' 'prototype'.
const Type& TypeAnnotationTransformer::ComputeThisTypeFromMember(
    const ast::Node& node) {
  if (!IsPrototypeProperty(node))
    return void_type();
  if (const auto* type = context().TryTypeOf(node))
    return *type;
  AddError(node, ErrorCode::JSDOC_EXPECT_TYPE);
  return void_type();
}

std::vector<const TypeParameter*>
TypeAnnotationTransformer::ComputeTypeParameters() {
  std::vector<const TypeParameter*> type_parameters;
  for (const auto& type_name : annotation_.type_parameter_names())
    type_parameters.push_back(
        &context().TypeOf(*type_name).As<TypeParameter>());
  return std::move(type_parameters);
}

TypeAnnotationTransformer::FunctionParameters
TypeAnnotationTransformer::ProcessParameterList(
    const ast::Node& parameter_list) {
  DCHECK_EQ(parameter_list, ast::SyntaxCode::ParameterList);
  return FunctionParameters::Builder(&context(), parameter_list,
                                     annotation_.parameter_tags())
      .Build();
}

TypeAnnotationTransformer::FunctionParameters
TypeAnnotationTransformer::ProcessParameterTags() {
  enum class State {
    Optional,
    Required,
    Rest,
  } state = State::Required;
  FunctionTypeArity arity;
  std::vector<const Type*> parameter_types;
  std::vector<const ast::Node*> parameter_names;
  for (const auto& parameter_tag : annotation_.parameter_tags()) {
    const auto& type_node = parameter_tag->child_at(1);
    const auto is_optional = type_node.Is<ast::OptionalType>();
    const auto is_rest = type_node.Is<ast::RestType>();
    if (!type_node.syntax().Is<ast::Type>())
      AddError(type_node, ErrorCode::JSDOC_EXPECT_TYPE);
    const auto& type =
        type_node.syntax().Is<ast::Type>()
            ? TransformType(is_optional || is_rest ? type_node.child_at(0)
                                                   : type_node)
            : unspecified_type();
    switch (state) {
      case State::Required:
        if (is_optional) {
          state = State::Optional;
          arity.maximum = arity.minimum + 1;
          break;
        }
        if (is_rest) {
          arity.has_rest = true;
          state = State::Rest;
          break;
        }
        ++arity.maximum;
        arity.minimum = arity.maximum;
        break;
      case State::Optional:
        if (is_rest) {
          arity.has_rest = true;
          state = State::Rest;
          break;
        }
        ++arity.maximum;
        break;
      case State::Rest:
        AddError(type_node, ErrorCode::JSDOC_UNEXPECT_REST);
        break;
      default:
        NOTREACHED() << "Invalid state " << static_cast<int>(state);
        break;
    }
    parameter_types.push_back(&type);
    const auto& reference = parameter_tag->child_at(2);
    if (!reference.Is<ast::ReferenceExpression>())
      continue;
    const auto& name = ast::ReferenceExpression::NameOf(reference);
    const auto name_id = ast::Name::KindOf(name);
    const auto& it =
        std::find_if(parameter_names.begin(), parameter_names.end(),
                     [&](const ast::Node* present) {
                       return ast::Name::KindOf(*present) == name_id;
                     });
    if (it == parameter_names.end())
      continue;
    AddError(name, ErrorCode::JSDOC_MULTIPLE_PARAMETER, **it);
  }
  return FunctionParameters(arity, parameter_types);
}

const Type& TypeAnnotationTransformer::ResolveTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return context().TypeOf(name);
}

// Transform @param, @return style function type to Type object.
const Type& TypeAnnotationTransformer::TransformAsFunctionType() {
  if (node_.Is<ast::Method>()) {
    const auto& this_type = ComputeThisType();
    const auto& parameters = FunctionParameters::Builder(
                                 &context(), ast::Method::ParametersOf(node_),
                                 annotation_.parameter_tags())
                                 .Build();
    const auto method_kind = ast::Method::MethodKindOf(node_);
    const auto& method_name = ast::Method::NameOf(node_);
    switch (method_kind) {
      case ast::MethodKind::NonStatic:
        DCHECK(this_type.Is<ClassType>()) << node_ << ':' << this_type;
        if (method_name == ast::TokenKind::Constructor) {
          return type_factory().NewFunctionType(
              FunctionTypeKind::Constructor, ComputeTypeParameters(),
              parameters.arity(), parameters.types(), this_type, this_type);
        }
        return type_factory().NewFunctionType(
            FunctionTypeKind::Normal, ComputeTypeParameters(),
            parameters.arity(), parameters.types(), ComputeReturnType(),
            this_type);
      case ast::MethodKind::Static:
        DCHECK_EQ(this_type, void_type()) << node_;
        // Note: It is OK to name method to |Constructor|, e.g.
        // class Foo { static constructor() {} }
        return type_factory().NewFunctionType(
            FunctionTypeKind::Normal, ComputeTypeParameters(),
            parameters.arity(), parameters.types(), ComputeReturnType(),
            void_type());
    }
    NOTREACHED() << "Unknown MethodKind " << static_cast<int>(method_kind);
    return void_type();
  }

  // Short hand of single parameter or no parameter function, e.g.
  // /**
  //  * @param {number} x
  //  * var foo;
  //  */
  // is equivalent to:
  // /**
  //  * @param {number} x
  //  * var foo = function(x) {};
  //  */
  auto parameters =
      node_.Is<ast::Function>()
          ? std::move(ProcessParameterList(ast::Function::ParametersOf(node_)))
          : std::move(ProcessParameterTags());

  if (annotation_.is_function()) {
    return type_factory().NewFunctionType(
        FunctionTypeKind::Normal, ComputeTypeParameters(), parameters.arity(),
        parameters.types(), ComputeReturnType(), ComputeThisType());
  }

  if (annotation_.is_constructor()) {
    const auto& class_type = ComputeThisType();
    return type_factory().NewFunctionType(
        FunctionTypeKind::Constructor, ComputeTypeParameters(),
        parameters.arity(), parameters.types(), class_type, class_type);
  }

  return void_type();
}

const Type& TypeAnnotationTransformer::TransformAsInterface() {
  const auto* const class_value = TryClassValueOf(node_);
  if (!class_value) {
    AddError(node_, ErrorCode::JSDOC_UNEXPECT_TAG);
    return unspecified_type();
  }
  return type_factory().NewClassType(*class_value);
}

const Type& TypeAnnotationTransformer::TransformType(const ast::Node& node) {
  DCHECK(node.syntax().Is<ast::Type>()) << node;
  return TypeTransformer(&context()).Transform(node);
}

const Class* TypeAnnotationTransformer::TryClassValueOf(
    const ast::Node& node) const {
  if (node.Is<ast::Class>())
    return &context().ValueOf(node).As<Class>();
  if (node.Is<ast::Function>())
    return context().ValueOf(node).TryAs<Class>();
  if (node.Is<ast::BindingNameElement>()) {
    const auto& variable = context().ValueOf(node).As<Variable>();
    if (variable.assignments().size() != 1)
      return nullptr;
    const auto& initializer = ast::BindingNameElement::InitializerOf(node);
    auto* const variable_value = context().TryValueOf(initializer);
    return variable_value ? variable_value->TryAs<Class>() : nullptr;
  }
  if (node.Is<ast::MemberExpression>()) {
    const auto* const value = context().TryValueOf(node);
    if (!value || !value->Is<Property>())
      return nullptr;
    const auto& property = value->As<Property>();
    if (property.assignments().size() != 1)
      return nullptr;
    auto* const property_value =
        context().TryValueOf(*property.assignments().front());
    return property_value ? property_value->TryAs<Class>() : nullptr;
  }
  DVLOG(0) << "We should handle: " << node;
  return nullptr;
}
}  // namespace analyzer
}  // namespace joana

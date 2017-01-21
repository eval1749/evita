// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "joana/analyzer/annotation.h"

#include "base/logging.h"
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
// Annotation::FunctionParameters
//
class Annotation::FunctionParameters final {
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

Annotation::FunctionParameters::FunctionParameters(
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

Annotation::FunctionParameters::FunctionParameters(FunctionParameters&& other)
    : arity_(other.arity_), types_(std::move(other.types_)) {
  other.arity_ = FunctionTypeArity();
}

Annotation::FunctionParameters::FunctionParameters() = default;
Annotation::FunctionParameters::~FunctionParameters() = default;

//
// Annotation::FunctionParameters::Builder
//
class Annotation::FunctionParameters::Builder final : public ContextUser {
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

  const ast::Node* FindTag(const ast::Node& name) const;
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

Annotation::FunctionParameters::Builder::Builder(
    Context* context,
    const ast::Node& parameter_list,
    const std::vector<const ast::Node*>& parameter_tags)
    : ContextUser(context),
      parameter_list_(parameter_list),
      parameter_tags_(parameter_tags) {
  DCHECK_EQ(parameter_list, ast::SyntaxCode::ParameterList);
}

Annotation::FunctionParameters::Builder::~Builder() = default;

Annotation::FunctionParameters
Annotation::FunctionParameters::Builder::Build() {
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

const ast::Node* Annotation::FunctionParameters::Builder::FindTag(
    const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto name_id = ast::Name::KindOf(name);
  for (const auto& parameter_tag : parameter_tags_) {
    const auto& reference = parameter_tag->child_at(2);
    if (!reference.Is<ast::ReferenceExpression>())
      continue;
    if (ast::ReferenceExpression::NameOf(reference) == name_id)
      return parameter_tag;
  }
  return nullptr;
}

void Annotation::FunctionParameters::Builder::ProcessBindingNameElement(
    const ast::Node& node) {
  const ast::Node& name = ast::BindingNameElement::NameOf(node);
  const auto has_initializer = !ast::BindingNameElement::InitializerOf(node)
                                    .Is<ast::ElisionExpression>();
  const auto* const tag = FindTag(name);
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

void Annotation::FunctionParameters::Builder::ProcessBindingRestElement(
    const ast::Node& node) {
  if (state_ == State::Rest)
    AddError(node, ErrorCode::JSDOC_UNEXPECT_REST);
  arity_.has_rest = true;
  const auto& binding = node.child_at(0);
  if (binding.Is<ast::BindingNameElement>()) {
    const auto& name = ast::BindingNameElement::NameOf(binding);
    const auto& maybe_tag = FindTag(name);
    if (!maybe_tag) {
      RecordName(name);
      parameter_types_.push_back(&unspecified_type());
      return;
    }
    const auto& tag = *maybe_tag;
    if (RecordName(name))
      RecordTag(tag);
    if (tag.Is<ast::OptionalType>())
      AddError(tag, ErrorCode::JSDOC_UNEXPECT_OPTIONAL);
    else if (tag.Is<ast::RestType>())
      AddError(tag, ErrorCode::JSDOC_UNEXPECT_REST);
    parameter_types_.push_back(&TransformType(tag.child_at(1)));
    return;
  }
  NOTREACHED() << "NYI parameter binding" << static_cast<int>(state_);
}

bool Annotation::FunctionParameters::Builder::RecordName(
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

void Annotation::FunctionParameters::Builder::RecordTag(const ast::Node& tag) {
  DCHECK_EQ(ast::JsDocTag::NameOf(tag), ast::TokenKind::AtParam);
  const auto& result = used_tags_.emplace(&tag);
  DCHECK(result.second) << tag;
}

const Type& Annotation::FunctionParameters::Builder::TransformType(
    const ast::Node& node) {
  return TypeTransformer(&context()).Transform(node);
}

//
// Annotation
//
Annotation::Annotation(Context* context,
                       const ast::Node& document,
                       const ast::Node& node,
                       const Type* this_type)
    : ContextUser(context),
      document_(document),
      node_(node),
      this_type_(this_type) {
  DCHECK_EQ(document_, ast::SyntaxCode::JsDocDocument);
  DCHECK(!this_type || this_type->Is<ClassType>()) << this_type;
}

Annotation::~Annotation() = default;

const ast::Node* Annotation::Classify() {
  for (const auto& node : ast::NodeTraversal::ChildNodesOf(document_)) {
    if (node != ast::SyntaxCode::JsDocTag)
      continue;
    const auto& tag_name = ast::JsDocTag::NameOf(node);
    switch (ast::Name::KindOf(tag_name)) {
      case ast::TokenKind::AtConst:
        RememberTag(&const_tag_, node);
        if (node.arity() == 1)
          continue;
        RememberTag(&kind_tag_, node);
        if (type_node_)
          type_node_ = &node.child_at(1);
        continue;
      case ast::TokenKind::AtConstructor:
      case ast::TokenKind::AtDict:
      case ast::TokenKind::AtEnum:
      case ast::TokenKind::AtInterface:
      case ast::TokenKind::AtRecord:
        RememberTag(&kind_tag_, node);
        continue;
      case ast::TokenKind::AtDefine:
      case ast::TokenKind::AtType:
      case ast::TokenKind::AtTypeDef:
        RememberTag(&kind_tag_, node);
        if (!type_node_)
          type_node_ = &node.child_at(1);
        continue;
      case ast::TokenKind::AtExtends:
        extends_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtFinal:
        RememberTag(&final_tag_, node);
        continue;
      case ast::TokenKind::AtImplements:
        implements_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtOverride:
        RememberTag(&override_tag_, node);
        continue;
      case ast::TokenKind::AtPrivate:
      case ast::TokenKind::AtProtected:
      case ast::TokenKind::AtPublic:
        RememberTag(&access_tag_, node);
        continue;
      case ast::TokenKind::AtParam:
        parameter_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtReturn:
        RememberTag(&return_tag_, node);
        continue;
      case ast::TokenKind::AtThis:
        RememberTag(&this_tag_, node);
        continue;
      case ast::TokenKind::AtTemplate:
        ProcessTemplateTag(node);
        continue;
    }
  }
  return kind_tag_ ? &ast::JsDocTag::NameOf(*kind_tag_) : nullptr;
}

const Type* Annotation::Compile() {
  const auto* kind = Classify();
  if (kind == nullptr) {
    if (!parameter_tags_.empty() || return_tag_)
      return &TransformAsFunctionType();
    MarkNotTypeAnnotation();
    return nullptr;
  }
  if (*kind == ast::TokenKind::AtConstructor)
    return &TransformAsFunctionType();
  if (*kind == ast::TokenKind::AtDict) {
    MarkNotTypeAnnotation();
    return nullptr;
  }
  if (*kind == ast::TokenKind::AtEnum) {
    // TODO(eval1749): NYI: enum type.
    MarkNotTypeAnnotation();
    return nullptr;
  }
  if (*kind == ast::TokenKind::AtInterface ||
      *kind == ast::TokenKind::AtRecord) {
    return &TransformAsInterface();
  }
  if (type_node_)
    return &TransformType(*type_node_);
  MarkNotTypeAnnotation();
  return nullptr;
}

const Type& Annotation::ComputeReturnType() {
  if (!return_tag_)
    return void_type();
  return TransformType(return_tag_->child_at(1));
}

// Type of "this" is determined by:
// - Class passed by caller during processing class declaration.
// - Class.prototype.Name;
// - Class.prototype[Expression];
const Type& Annotation::ComputeThisType() {
  return this_type_ ? *this_type_ : void_type();
}

// Returns type of |Expression| where |Expression| '.' 'prototype'.
const Type& Annotation::ComputeThisTypeFromMember(const ast::Node& node) {
  if (!IsPrototypeProperty(node))
    return void_type();
  if (const auto* type = context().TryTypeOf(node))
    return *type;
  AddError(node, ErrorCode::JSDOC_EXPECT_TYPE);
  return void_type();
}

// Note: We can't check whether @override tag is valid or invalid since we
// don't known class hierarchy yet.
void Annotation::MarkNotTypeAnnotation() {
  if (access_tag_)
    AddError(*access_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  for (const auto& extends_tag : extends_tags_)
    AddError(*extends_tag, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (final_tag_)
    AddError(*final_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  for (const auto& implements_tag : implements_tags_)
    AddError(*implements_tag, ErrorCode::JSDOC_UNEXPECT_TAG);
  for (const auto& parameter_tag : parameter_tags_)
    AddError(*parameter_tag, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (return_tag_)
    AddError(*return_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  for (const auto& template_tag : template_tags_)
    AddError(*template_tag, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (this_tag_)
    AddError(*this_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
}

Annotation::FunctionParameters Annotation::ProcessParameterList(
    const ast::Node& parameter_list) {
  DCHECK_EQ(parameter_list, ast::SyntaxCode::ParameterList);
  return FunctionParameters::Builder(&context(), parameter_list,
                                     parameter_tags_)
      .Build();
}

Annotation::FunctionParameters Annotation::ProcessParameterTags() {
  enum class State {
    Optional,
    Required,
    Rest,
  } state = State::Required;
  FunctionTypeArity arity;
  std::vector<const Type*> parameter_types;
  std::vector<const ast::Node*> parameter_names;
  for (const auto& parameter_tag : parameter_tags_) {
    const auto& type_node = parameter_tag->child_at(1);
    const auto is_optional = type_node.Is<ast::OptionalType>();
    const auto is_rest = type_node.Is<ast::RestType>();
    const auto& type = TransformType(
        is_optional || is_rest ? type_node.child_at(0) : type_node);
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
    context().RegisterType(reference, type);
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

void Annotation::RememberTag(const ast::Node** pointer, const ast::Node& node) {
  if (*pointer) {
    AddError(node, ErrorCode::JSDOC_MULTIPLE_TAG, **pointer);
    return;
  }
  *pointer = &node;
}

void Annotation::ProcessTemplateTag(const ast::Node& node) {
  DCHECK_EQ(ast::JsDocTag::NameOf(node), ast::TokenKind::AtTemplate);
  template_tags_.push_back(&node);
  for (const auto& operand : ast::JsDocTag::OperandsOf(node)) {
    if (!operand.Is<ast::TypeName>())
      continue;
    const auto& name = ast::TypeName::NameOf(operand);
    if (!name.Is<ast::Name>()) {
      AddError(name, ErrorCode::JSDOC_EXPECT_NAME);
      continue;
    }
    const auto name_id = ast::Name::KindOf(name);
    const auto& result = type_parameter_map_.emplace(
        name_id, &type_factory().NewTypeParameter(name).As<TypeParameter>());
    if (result.second)
      continue;
    for (const auto& present : ast::JsDocTag::OperandsOf(node)) {
      if (present != name_id)
        continue;
      AddError(name, ErrorCode::JSDOC_MULTIPLE_NAME, present);
      break;
    }
  }
}

const Type& Annotation::ResolveTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return context().TypeOf(name);
}

// Transform @param, @return style function type to Type object.
const Type& Annotation::TransformAsFunctionType() {
  if (node_.Is<ast::Method>()) {
    const auto& this_type = ComputeThisType();
    DCHECK(this_type.Is<ClassType>()) << this_type;
    if (kind_tag_)
      AddError(*kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
    const auto& parameters =
        FunctionParameters::Builder(
            &context(), ast::Method::ParametersOf(node_), parameter_tags_)
            .Build();
    const auto method_kind = ast::Method::MethodKindOf(node_);
    const auto& method_name = ast::Method::NameOf(node_);
    switch (method_kind) {
      case ast::MethodKind::NonStatic:
        if (method_name == ast::TokenKind::Constructor) {
          return type_factory().NewFunctionType(
              FunctionTypeKind::Constructor, type_parameters_,
              parameters.arity(), parameters.types(), this_type, this_type);
        }
        return type_factory().NewFunctionType(
            FunctionTypeKind::Normal, type_parameters_, parameters.arity(),
            parameters.types(), ComputeReturnType(), this_type);
      case ast::MethodKind::Static:
        // Note: It is OK to name method to |Constructor|, e.g.
        // class Foo { static constructor() {} }
        return type_factory().NewFunctionType(
            FunctionTypeKind::Normal, type_parameters_, parameters.arity(),
            parameters.types(), ComputeReturnType(), void_type());
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

  if (!kind_tag_) {
    return type_factory().NewFunctionType(
        FunctionTypeKind::Normal, type_parameters_, parameters.arity(),
        parameters.types(), ComputeReturnType(), ComputeThisType());
  }

  if (ast::JsDocTag::NameOf(*kind_tag_) == ast::TokenKind::AtConstructor) {
    const auto& class_type = ComputeThisType();
    return type_factory().NewFunctionType(
        FunctionTypeKind::Constructor, type_parameters_, parameters.arity(),
        parameters.types(), class_type, class_type);
  }

  AddError(*kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  return void_type();
}

const Type& Annotation::TransformAsInterface() {
  auto* const class_value = TryClassValueOf(node_);
  if (!class_value) {
    AddError(node_, ErrorCode::JSDOC_UNEXPECT_TAG);
    return unspecified_type();
  }
  return type_factory().NewClassType(class_value);
}

const Type& Annotation::TransformType(const ast::Node& node) {
  DCHECK(node.syntax().Is<ast::Type>()) << node;
  return TypeTransformer(&context()).Transform(node);
}

Class* Annotation::TryClassValueOf(const ast::Node& node) const {
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
  NOTREACHED() << node;
  return nullptr;
}
}  // namespace analyzer
}  // namespace joana

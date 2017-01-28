// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>
#include <utility>

#include "joana/analyzer/annotation_compiler.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
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

bool CanHaveVisibility(const ast::Node& node) {
  if (node.Is<ast::Method>())
    return true;
  if (!node.Is<ast::ExpressionStatement>())
    return false;
  const auto& expression = ast::ExpressionStatement::ExpressionOf(node);
  return expression.Is<ast::ComputedMemberExpression>() ||
         expression.Is<ast::MemberExpression>();
}

const ast::Node* FindName(const ast::Node& name,
                          const std::vector<const ast::Node*>& names) {
  const auto name_id = ast::Name::KindOf(name);
  const auto& it =
      std::find_if(names.begin(), names.end(), [&](const ast::Node* type_name) {
        const auto& present = ast::TypeName::NameOf(*type_name);
        return ast::Name::KindOf(present) == name_id;
      });
  return it == names.end() ? nullptr : *it;
}

}  // namespace

using Compiler = Annotation::Compiler;

//
// Annotation::Compiler
//
Compiler::Compiler(const base::Optional<Context*>& maybe_context)
    : maybe_context_(maybe_context) {}

Compiler::~Compiler() = default;

// The entry point
Annotation Compiler::Compile(const ast::Node& document,
                             const ast::Node& target) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  annotation_.document_ = &document;
  Classify(document);
  if (!maybe_context_)
    return std::move(annotation_);
  Validate(target);
  return std::move(annotation_);
}

// Private members
void Compiler::AddError(const ast::Node& node,
                        ErrorCode error_code,
                        const ast::Node& related) {
  if (!maybe_context_)
    return;
  (*maybe_context_)->AddError(node, error_code, related);
}

void Compiler::AddError(const ast::Node& node, ErrorCode error_code) {
  if (!maybe_context_)
    return;
  (*maybe_context_)->AddError(node, error_code);
}

void Compiler::Classify(const ast::Node& document) {
  DCHECK_EQ(document, ast::SyntaxCode::JsDocDocument);
  for (const auto& node : ast::NodeTraversal::ChildNodesOf(document)) {
    if (node != ast::SyntaxCode::JsDocTag)
      continue;
    const auto& tag_name = ast::JsDocTag::NameOf(node);
    switch (ast::Name::KindOf(tag_name)) {
      case ast::TokenKind::AtConst:
        RememberTag(&annotation_.const_tag_, node);
        if (node.arity() == 1)
          continue;
        annotation_.kind_ = Kind::Type;
        RememberTag(&annotation_.kind_tag_, node);
        RememberType(node);
        continue;
      case ast::TokenKind::AtConstructor:
        annotation_.kind_ = Kind::Constructor;
        RememberTag(&annotation_.kind_tag_, node);
        continue;
      case ast::TokenKind::AtDict:
        annotation_.kind_ = Kind::Dict;
        RememberTag(&annotation_.kind_tag_, node);
        continue;
      case ast::TokenKind::AtEnum:
        annotation_.kind_ = Kind::Enum;
        RememberTag(&annotation_.kind_tag_, node);
        continue;
      case ast::TokenKind::AtInterface:
      case ast::TokenKind::AtRecord:
        annotation_.kind_ = Kind::Interface;
        RememberTag(&annotation_.kind_tag_, node);
        continue;
      case ast::TokenKind::AtDefine:
        annotation_.kind_ = Kind::Define;
        RememberTag(&annotation_.kind_tag_, node);
        RememberType(node);
        continue;
      case ast::TokenKind::AtType:
        annotation_.kind_ = Kind::Type;
        RememberTag(&annotation_.kind_tag_, node);
        RememberType(node);
        continue;
      case ast::TokenKind::AtTypeDef:
        annotation_.kind_ = Kind::TypeDef;
        RememberTag(&annotation_.kind_tag_, node);
        RememberType(node);
        continue;
      case ast::TokenKind::AtExtends:
        annotation_.extends_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtFinal:
        RememberTag(&annotation_.final_tag_, node);
        continue;
      case ast::TokenKind::AtImplements:
        annotation_.implements_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtOverride:
        RememberTag(&annotation_.override_tag_, node);
        continue;
      case ast::TokenKind::AtPrivate:
      case ast::TokenKind::AtProtected:
      case ast::TokenKind::AtPublic:
        RememberTag(&annotation_.access_tag_, node);
        continue;
      case ast::TokenKind::AtParam:
        annotation_.parameter_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtReturn:
        RememberTag(&annotation_.return_tag_, node);
        continue;
      case ast::TokenKind::AtThis:
        RememberTag(&annotation_.this_tag_, node);
        continue;
      case ast::TokenKind::AtTemplate:
        ProcessTemplateTag(node);
        continue;
    }
  }
}

void Compiler::ProcessTemplateTag(const ast::Node& node) {
  DCHECK_EQ(ast::JsDocTag::NameOf(node), ast::TokenKind::AtTemplate);
  for (const auto& operand : ast::JsDocTag::OperandsOf(node)) {
    if (!operand.Is<ast::TypeName>())
      continue;
    const auto& name = ast::TypeName::NameOf(operand);
    if (!name.Is<ast::Name>()) {
      AddError(name, ErrorCode::JSDOC_EXPECT_NAME);
      continue;
    }
    const auto* present = FindName(name, annotation_.type_parameter_names_);
    if (!present) {
      annotation_.type_parameter_names_.push_back(&operand);
      continue;
    }
    AddError(node, ErrorCode::JSDOC_MULTIPLE_NAME, *present);
  }
}

void Compiler::RememberTag(const ast::Node** pointer, const ast::Node& node) {
  if (*pointer) {
    AddError(node, ErrorCode::JSDOC_MULTIPLE_TAG, **pointer);
    return;
  }
  *pointer = &node;
}

void Compiler::RememberType(const ast::Node& node) {
  if (annotation_.type_node_)
    return;
  annotation_.type_node_ = &node.child_at(1);
}

void Compiler::ReportErrorTag(ErrorCode error_code,
                              const ast::Node* maybe_tag) {
  DCHECK(maybe_context_);
  if (!maybe_tag)
    return;
  AddError(*maybe_tag, error_code);
}

void Compiler::ReportErrorTags(ErrorCode error_code,
                               const std::vector<const ast::Node*>& tags) {
  DCHECK(maybe_context_);
  for (const auto& tag : tags)
    AddError(*tag, error_code);
}

void Compiler::Validate(const ast::Node& target) {
  DCHECK(maybe_context_);
  if (annotation_.access_tag_ && !CanHaveVisibility(target)) {
    AddError(*annotation_.access_tag_, ErrorCode::JSDOC_UNEXPECT_VISIBILITY,
             target);
  }
  if (!annotation_.kind_tag_) {
    if (annotation_.is_function())
      return ValidateForType(target);
    return ValidateForFunction(target);
  }
  if (annotation_.is_constructor())
    return ValidateForConstructor(target);
  if (annotation_.is_interface())
    return ValidateForInterface(target);
  if (annotation_.is_function())
    return ValidateForFunction(target);
  return ValidateForType(target);
}

void Compiler::ValidateForConstructor(const ast::Node& target) {
  if (annotation_.extends_tags_.size() > 1) {
    ReportErrorTags(ErrorCode::JSDOC_UNEXPECT_TAG,
                    std::vector<const ast::Node*>(
                        std::next(annotation_.extends_tags_.begin()),
                        annotation_.extends_tags_.end()));
  }
}

void Compiler::ValidateForEnum(const ast::Node& target) {
  if (CanHaveVisibility(target))
    AddError(*annotation_.kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  ReportErrorTags(ErrorCode::JSDOC_UNEXPECT_TAG, annotation_.parameter_tags_);
  ReportErrorTag(ErrorCode::JSDOC_UNEXPECT_TAG, annotation_.return_tag_);
}

void Compiler::ValidateForFunction(const ast::Node& target) {}

void Compiler::ValidateForInterface(const ast::Node& target) {
  ReportErrorTags(ErrorCode::JSDOC_UNEXPECT_TAG, annotation_.parameter_tags_);
  ReportErrorTag(ErrorCode::JSDOC_UNEXPECT_TAG, annotation_.return_tag_);
}

void Compiler::ValidateForType(const ast::Node& target) {
  if (target.Is<ast::Class>())
    AddError(*annotation_.kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
}

}  // namespace analyzer
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "joana/analyzer/annotation.h"

#include "joana/analyzer/values.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Annotation
//
Annotation::Annotation(Annotation&& other)
    : access_tag_(other.access_tag_),
      const_tag_(other.const_tag_),
      document_(other.document_),
      extends_tags_(std::move(other.extends_tags_)),
      final_tag_(other.final_tag_),
      implements_tags_(std::move(other.implements_tags_)),
      kind_(other.kind_),
      kind_tag_(other.kind_tag_),
      override_tag_(other.override_tag_),
      parameter_tags_(std::move(other.parameter_tags_)),
      return_tag_(other.return_tag_),
      this_tag_(other.this_tag_),
      type_node_(other.type_node_),
      type_parameter_names_(std::move(other.type_parameter_names_)) {
  other.access_tag_ = nullptr;
  other.const_tag_ = nullptr;
  other.final_tag_ = nullptr;
  other.override_tag_ = nullptr;
  other.return_tag_ = nullptr;
  other.this_tag_ = nullptr;
  other.type_node_ = nullptr;
}

Annotation::Annotation() = default;
Annotation::~Annotation() = default;

ClassKind Annotation::class_kind() const {
  if (!kind_tag_)
    return ClassKind::Class;
  const auto& name = ast::JsDocTag::NameOf(*kind_tag_);
  if (name == ast::TokenKind::AtConstructor)
    return ClassKind::Class;
  if (name == ast::TokenKind::AtInterface)
    return ClassKind::Interface;
  if (name == ast::TokenKind::AtRecord)
    return ClassKind::Record;
  NOTREACHED() << "Expect @constructor, @Interface or @record " << *kind_tag_;
  return ClassKind::Class;
}

const ast::Node& Annotation::document() const {
  DCHECK(document_);
  return *document_;
}

Visibility Annotation::visibility() const {
  if (!access_tag_)
    return Visibility::Public;
  const auto& name = ast::JsDocTag::NameOf(*access_tag_);
  if (name == ast::TokenKind::AtPrivate)
    return Visibility::Private;
  if (name == ast::TokenKind::AtProtected)
    return Visibility::Protected;
  if (name == ast::TokenKind::AtPublic)
    return Visibility::Public;
  NOTREACHED() << "Unknown access tag " << *access_tag_;
  return Visibility::Public;
}

}  // namespace analyzer
}  // namespace joana

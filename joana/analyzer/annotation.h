// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANNOTATION_H_
#define JOANA_ANALYZER_ANNOTATION_H_

#include <unordered_map>
#include <vector>

#include "joana/analyzer/context_user.h"

namespace joana {

namespace ast {
class Node;
enum class TokenKind;
}

namespace analyzer {

class Context;
enum class ErrorCode;
class Factory;
class Type;
class TypeNameResolver;
class TypeParameter;
class Value;

//
// Annotation represents compiled annotation.
//
class Annotation final : public ContextUser {
 public:
  // |node| is |ast::Annotation|.
  // |this_type| is |nullptr| or |ClassType| for class declaration.
  Annotation(Context* context,
             TypeNameResolver* resolver,
             const Type* this_type,
             const ast::Node& node);
  ~Annotation();

  bool has_type() const { return type_ != nullptr; }
  const ast::Node& kind() const;
  const ast::Node& node() const { return node_; }
  const Type& type() const;

  void Compile();
  bool IsAnnotated(const ast::Node& node) const;

 private:
  const ast::Node* Classify();

  const Type& ComputeReturnType();
  const Type& ComputeThisType();
  const Type& ComputeThisTypeFromMember(const ast::Node& node);

  void MarkNotTypeAnnotation();

  void ProcessParameter(std::vector<const ast::Node*>* parameter_names,
                        std::vector<const Type*>* parameter_types,
                        const ast::Node& parameter_node);

  std::vector<const Type*> Annotation::ProcessParameterList(
      const ast::Node& parameter_list);

  void RememberTag(const ast::Node** pointer, const ast::Node& node);

  const Type& ResolveTypeName(const ast::Node& name);

  const Type& ToGenericTypeIfNeeded(const Type& type);
  const Type& TransformAsFunctionType();
  const Type& TransformAsInterface();

  // Transform AST type node to Type object.
  const Type& TransformType(const ast::Node& node);

  const ast::Node& node_;
  TypeNameResolver& resolver_;
  const Type* this_type_;
  const Type* type_ = nullptr;
  std::unordered_map<ast::TokenKind, const TypeParameter*> type_parameter_map_;
  std::vector<const TypeParameter*> type_parameters_;

  // Tags
  const ast::Node* access_tag_ = nullptr;
  const ast::Node* const_tag_ = nullptr;
  const ast::Node* extends_tag_ = nullptr;
  const ast::Node* final_tag_;
  const ast::Node* implements_tag_ = nullptr;
  const ast::Node* kind_tag_ = nullptr;
  const ast::Node* override_tag_ = nullptr;
  std::vector<const ast::Node*> parameter_tags_;
  const ast::Node* return_tag_ = nullptr;
  const ast::Node* template_tag_ = nullptr;
  const ast::Node* this_tag_ = nullptr;
  const ast::Node* type_node_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Annotation);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANNOTATION_H_

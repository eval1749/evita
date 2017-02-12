// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_ANNOTATION_TRANSFORMER_H_
#define JOANA_ANALYZER_TYPE_ANNOTATION_TRANSFORMER_H_

#include <unordered_map>
#include <vector>

#include "joana/analyzer/context_user.h"

namespace joana {

namespace ast {
class Node;
enum class TokenKind;
}

namespace analyzer {

class Annotation;
class Class;
class Context;
enum class ErrorCode;
class Factory;
class Type;
class TypeParameter;
class Value;

//
// TypeAnnotationTransformer represents compiled annotation.
//
class TypeAnnotationTransformer final : public ContextUser {
 public:
  // |node| is |ast::TypeAnnotationTransformer|.
  // |this_type| is |nullptr| or |ClassType| for class declaration.
  TypeAnnotationTransformer(Context* context,
                            const Annotation& annotation,
                            const ast::Node& node,
                            const Type* this_type = nullptr);
  ~TypeAnnotationTransformer();

  const Type* Compile();

 private:
  class FunctionParameters;

  const Type& ComputeReturnType();
  const Type& ComputeThisType();
  const Type& ComputeThisTypeFromMember(const ast::Node& node);
  std::vector<const TypeParameter*> ComputeTypeParameters();

  FunctionParameters ProcessParameterList(const ast::Node& parameter_list);

  FunctionParameters ProcessParameterTags();

  const Type& ResolveTypeName(const ast::Node& name);

  const Type& TransformAsFunctionType();
  const Type& TransformAsInterface();
  const Type& TransformType(const ast::Node& node);

  // Return |Class| value associated to |node|
  const Class* TryClassValueOf(const ast::Node& node) const;

  const Annotation& annotation_;
  const ast::Node& node_;
  const Type* this_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeAnnotationTransformer);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_ANNOTATION_TRANSFORMER_H_

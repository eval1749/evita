// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_RESOLVER_H_
#define JOANA_ANALYZER_TYPE_RESOLVER_H_

#include <vector>

#include "joana/analyzer/pass.h"

#include "joana/ast/syntax_forward.h"
#include "joana/ast/syntax_visitor.h"

namespace joana {
namespace analyzer {

class Annotation;
class Class;
class Type;
class Value;

//
// TypeResolver
//
class TypeResolver final : public Pass, public ast::SyntaxVisitor {
 public:
  explicit TypeResolver(Context* context);
  ~TypeResolver() final;

  void RunOn(const ast::Node& node) final;

 private:
  std::vector<Class*> ComputeClassHeritage(const Annotation& annotation,
                                           const ast::Node& node);
  const Type* ComputeClassType(const ast::Node& node) const;
  const Type* ComputeType(const Annotation& annotation,
                          const ast::Node& node,
                          const Type* maybe_this_type);

  void ProcessAnnotation(const ast::Node& node,
                         const Annotation& annotation,
                         const Type* maybe_this_type);
  void ProcessArrayBinding(const ast::Node& node, const Type& type);
  void ProcessAssignment(const ast::Node& node, const Annotation& annotation);
  void ProcessBinding(const ast::Node& node, const Type& type);
  void ProcessClass(const ast::Node& node, const Annotation& annotation);
  void ProcessFunction(const ast::Node& node, const Annotation& annotation);
  void ProcessObjectBinding(const ast::Node& node, const Type& type);
  void ProcessVariableDeclaration(const ast::Node& node,
                                  const Annotation& annotation);
  void RegisterType(const ast::Node& node, const Type& type);
  Class* ResolveClass(const ast::Node& node);
  Value* SingleVariableValueOf(const ast::Node& node) const;

  // |ast::SyntaxVisitor| members
  void VisitDefault(const ast::Node& node) final;
  void VisitInternal(const ast::Annotation& syntax,
                     const ast::Node& node) final;

  void VisitInternal(const ast::Class& syntax, const ast::Node& node) final;

  DISALLOW_COPY_AND_ASSIGN(TypeResolver);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_RESOLVER_H_

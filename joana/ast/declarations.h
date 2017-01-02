// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_DECLARATIONS_H_
#define JOANA_AST_DECLARATIONS_H_

#include <iosfwd>

#include "joana/ast/node.h"
#include "joana/ast/syntax.h"
#include "joana/ast/syntax_forward.h"

namespace joana {
namespace ast {

//
// FunctionKind
//
#define FOR_EACH_AST_FUNCTION_KIND(V) \
  V(Invalid)                          \
  V(Async)                            \
  V(Generator)                        \
  V(Getter)                           \
  V(Normal)                           \
  V(Setter)

enum class FunctionKind {
#define V(name) name,
  FOR_EACH_AST_FUNCTION_KIND(V)
#undef V
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          FunctionKind kind);

//
// MethodKind
//
#define FOR_EACH_AST_METHOD_KIND(V) \
  V(Constructor)                    \
  V(NonStatic)                      \
  V(Static)

enum class MethodKind {
#define V(name) name,
  FOR_EACH_AST_METHOD_KIND(V)
#undef V
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          MethodKind kind);

//
// AnnotationSyntax
//
class JOANA_AST_EXPORT AnnotationSyntax final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(Annotation, Syntax);

 public:
  ~AnnotationSyntax() final;

  static const Node& AnnotatedOf(const Node& node);
  static const Node& AnnotationOf(const Node& node);

 private:
  AnnotationSyntax();

  DISALLOW_COPY_AND_ASSIGN(AnnotationSyntax);
};

//
// ArrowFunctionSyntax
//
class JOANA_AST_EXPORT ArrowFunctionSyntax final
    : public SyntaxTemplate<Syntax, FunctionKind> {
  DECLARE_CONCRETE_AST_SYNTAX(ArrowFunction, Syntax);

 public:
  ~ArrowFunctionSyntax() final;

  static const Node& BodyOf(const Node& node);

  //  - x = ReferenceExpression
  //  - () = ParameterList
  //  - (a, b, ...) = ParameterList
  static const Node& ParametersOf(const Node& node);

 private:
  explicit ArrowFunctionSyntax(FunctionKind kind);

  DISALLOW_COPY_AND_ASSIGN(ArrowFunctionSyntax);
};

//
// ClassSyntax
//
class JOANA_AST_EXPORT ClassSyntax final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(Class, Syntax);

 public:
  ~ClassSyntax() final;

  static const Node& BodyOf(const Node& node);
  static const Node& HerisyntaxeOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  ClassSyntax();

  DISALLOW_COPY_AND_ASSIGN(ClassSyntax);
};

//
// FunctionSyntax
//
class JOANA_AST_EXPORT FunctionSyntax final
    : public SyntaxTemplate<Syntax, FunctionKind> {
  DECLARE_CONCRETE_AST_SYNTAX(Function, Syntax);

 public:
  ~FunctionSyntax() final;

  FunctionKind kind() const { return parameter_at<0>(); }

  // Returns |BlockStatement|.
  static const Node& BodyOf(const Node& node);

  // Returns |Name| or |Empty|.
  static const Node& NameOf(const Node& node);

  // Returns |ParameterList| node.
  static const Node& ParametersOf(const Node& node);

 private:
  explicit FunctionSyntax(FunctionKind kind);

  DISALLOW_COPY_AND_ASSIGN(FunctionSyntax);
};

//
// MethodSyntax
//
class JOANA_AST_EXPORT MethodSyntax final
    : public SyntaxTemplate<Syntax, MethodKind, FunctionKind> {
  DECLARE_CONCRETE_AST_SYNTAX(Method, Syntax);

 public:
  ~MethodSyntax() final;

  FunctionKind kind() const { return parameter_at<1>(); }
  MethodKind method_kind() const { return parameter_at<0>(); }

  // Returns |BlockStatement|.
  static const Node& BodyOf(const Node& node);

  // Returns either |ArrayInitializer|, e.g. "[Symbol.foo]" or
  // |ReferenceExpression|.
  static const Node& NameOf(const Node& node);

  // Returns |ParameterList| node.
  static const Node& ParametersOf(const Node& node);

 private:
  MethodSyntax(MethodKind method_kind, FunctionKind kind);

  DISALLOW_COPY_AND_ASSIGN(MethodSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_DECLARATIONS_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_DECLARATIONS_H_
#define JOANA_AST_DECLARATIONS_H_

#include "joana/ast/node.h"
#include "joana/ast/syntax.h"
#include "joana/ast/syntax_forward.h"

namespace joana {
namespace ast {

//
// FunctionKind
//
enum class FunctionKind {
  Invalid,
  Async,
  Generator,
  Getter,
  Normal,
  Setter,
};

//
// MethodKind
//
enum class MethodKind {
  Constructor,
  NonStatic,
  Static,
};

//
// ArrowFunctionSyntax
//
class JOANA_AST_EXPORT ArrowFunctionSyntax final
    : public SyntaxTemplate<Syntax, FunctionKind> {
  DECLARE_CONCRETE_AST_SYNTAX(ArrowFunctionSyntax, Syntax);

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
  DECLARE_CONCRETE_AST_SYNTAX(ClassSyntax, Syntax);

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
  DECLARE_CONCRETE_AST_SYNTAX(FunctionSyntax, Syntax);

 public:
  ~FunctionSyntax() final;

  static const Node& BodyOf(const Node& node);
  static const Node& NameOf(const Node& node);
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
  DECLARE_CONCRETE_AST_SYNTAX(MethodSyntax, Syntax);

 public:
  ~MethodSyntax() final;

  static const Node& BodyOf(const Node& node);
  static const Node& NameOf(const Node& node);
  static const Node& ParametersOf(const Node& node);

 private:
  MethodSyntax(MethodKind method_kind, FunctionKind kind);

  DISALLOW_COPY_AND_ASSIGN(MethodSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_DECLARATIONS_H_

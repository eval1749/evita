// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_EXPRESSIONS_H_
#define JOANA_AST_EXPRESSIONS_H_

#include <vector>

#include "joana/ast/node.h"
#include "joana/ast/syntax.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"
#include "joana/base/iterator_utils.h"

namespace joana {
namespace ast {

DECLARE_AST_SYNTAX_0(ArgumentList)
DECLARE_AST_SYNTAX_0(ArrayInitializer)
DECLARE_AST_SYNTAX_0(CallExpression)
DECLARE_AST_SYNTAX_0(CommaExpression)
DECLARE_AST_SYNTAX_0(ComputedMemberExpression)
DECLARE_AST_SYNTAX_0(ConditionalExpression)
DECLARE_AST_SYNTAX_0(DelimiterExpression)
DECLARE_AST_SYNTAX_0(GroupExpression)
DECLARE_AST_SYNTAX_0(ElisionExpression)
DECLARE_AST_SYNTAX_0(NewExpression)
DECLARE_AST_SYNTAX_0(MemberExpression)
DECLARE_AST_SYNTAX_0(ObjectInitializer)
DECLARE_AST_SYNTAX_0(ParameterList)
DECLARE_AST_SYNTAX_0(Tuple);

//
// AssignmentExpressionSyntax
//
class JOANA_AST_EXPORT AssignmentExpressionSyntax final
    : public SyntaxTemplate<Syntax, PunctuatorKind> {
  DECLARE_CONCRETE_AST_SYNTAX(AssignmentExpressionSyntax, Syntax);

 public:
  ~AssignmentExpressionSyntax() final;

  static const Node& LeftHandSideOf(const Node& node);
  static const Node& OperatorOf(const Node& node);
  static const Node& RightHandSideOf(const Node& node);

 private:
  explicit AssignmentExpressionSyntax(PunctuatorKind kind);

  DISALLOW_COPY_AND_ASSIGN(AssignmentExpressionSyntax);
};

//
// BinaryExpressionSyntax
//
class JOANA_AST_EXPORT BinaryExpressionSyntax final
    : public SyntaxTemplate<Syntax, PunctuatorKind> {
  DECLARE_CONCRETE_AST_SYNTAX(BinaryExpressionSyntax, Syntax);

 public:
  ~BinaryExpressionSyntax() final;

  static const Node& LeftHandSideOf(const Node& node);
  static const Node& OperatorOf(const Node& node);
  static const Node& RightHandSideOf(const Node& node);

 private:
  explicit BinaryExpressionSyntax(PunctuatorKind kind);

  DISALLOW_COPY_AND_ASSIGN(BinaryExpressionSyntax);
};

//
// PropertySyntax
//
class JOANA_AST_EXPORT PropertySyntax final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(PropertySyntax, Syntax);

 public:
  ~PropertySyntax() final;

  static const Node& NameOf(const Node& node);
  static const Node& ValueOf(const Node& node);

 private:
  PropertySyntax();

  DISALLOW_COPY_AND_ASSIGN(PropertySyntax);
};

//
// ReferenceExpressionSyntax
//
class JOANA_AST_EXPORT ReferenceExpressionSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ReferenceExpressionSyntax, Syntax);

 public:
  ~ReferenceExpressionSyntax() final;

  static const Node& NameOf(const Node& node);

 private:
  ReferenceExpressionSyntax();

  DISALLOW_COPY_AND_ASSIGN(ReferenceExpressionSyntax);
};

//
// RegExpLiteralExpressionSyntax
//
class JOANA_AST_EXPORT RegExpLiteralExpressionSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpLiteralExpressionSyntax, Syntax);

 public:
  ~RegExpLiteralExpressionSyntax() final;

  static const Node& RegExpOf(const Node& node);
  static const Node& FlagsOf(const Node& node);

 private:
  RegExpLiteralExpressionSyntax();

  DISALLOW_COPY_AND_ASSIGN(RegExpLiteralExpressionSyntax);
};

//
// UnaryExpressionSyntax
//
class JOANA_AST_EXPORT UnaryExpressionSyntax final
    : public SyntaxTemplate<Syntax, PunctuatorKind> {
  DECLARE_CONCRETE_AST_SYNTAX(UnaryExpressionSyntax, Syntax);

 public:
  ~UnaryExpressionSyntax() final;

  static const Node& ExpressionOf(const Node& node);
  static const Node& OperatorOf(const Node& node);

 private:
  explicit UnaryExpressionSyntax(PunctuatorKind kind);

  DISALLOW_COPY_AND_ASSIGN(UnaryExpressionSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EXPRESSIONS_H_

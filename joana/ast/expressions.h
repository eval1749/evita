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

class ChildNodes;
class Node;

DECLARE_AST_SYNTAX_0(ArrayInitializer)
DECLARE_AST_SYNTAX_0(CommaExpression)
DECLARE_AST_SYNTAX_0(ConditionalExpression)
DECLARE_AST_SYNTAX_0(DelimiterExpression)
DECLARE_AST_SYNTAX_0(GroupExpression)
DECLARE_AST_SYNTAX_0(ElisionExpression)
DECLARE_AST_SYNTAX_0(ObjectInitializer)
DECLARE_AST_SYNTAX_0(ParameterList)
DECLARE_AST_SYNTAX_0(Tuple);

//
// AssignmentExpression
//
class JOANA_AST_EXPORT AssignmentExpression final
    : public SyntaxTemplate<Syntax, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(AssignmentExpression, Syntax);

 public:
  ~AssignmentExpression() final;

  TokenKind op() const { return parameter_at<0>(); }

  static const Node& LeftHandSideOf(const Node& node);
  static const Node& OperatorOf(const Node& node);
  static const Node& RightHandSideOf(const Node& node);

 private:
  explicit AssignmentExpression(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(AssignmentExpression);
};

//
// BinaryExpression
//
class JOANA_AST_EXPORT BinaryExpression final
    : public SyntaxTemplate<Syntax, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(BinaryExpression, Syntax);

 public:
  ~BinaryExpression() final;

  TokenKind op() const { return parameter_at<0>(); }

  static const Node& LeftHandSideOf(const Node& node);
  static const Node& OperatorOf(const Node& node);
  static const Node& RightHandSideOf(const Node& node);

 private:
  explicit BinaryExpression(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(BinaryExpression);
};

//
// CallExpression
//
class JOANA_AST_EXPORT CallExpression final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(CallExpression, Syntax);

 public:
  ~CallExpression() final;

  static ChildNodes ArgumentsOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  CallExpression();

  DISALLOW_COPY_AND_ASSIGN(CallExpression);
};

//
// ComputedMemberExpression
//
class JOANA_AST_EXPORT ComputedMemberExpression final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ComputedMemberExpression, Syntax);

 public:
  ~ComputedMemberExpression() final;

  static const Node& ExpressionOf(const Node& node);
  static const Node& MemberExpressionOf(const Node& node);

 private:
  ComputedMemberExpression();

  DISALLOW_COPY_AND_ASSIGN(ComputedMemberExpression);
};

//
// MemberExpression
//
class JOANA_AST_EXPORT MemberExpression final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(MemberExpression, Syntax);

 public:
  ~MemberExpression() final;

  static const Node& NameOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  MemberExpression();

  DISALLOW_COPY_AND_ASSIGN(MemberExpression);
};

//
// NewExpression
//
class JOANA_AST_EXPORT NewExpression final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(NewExpression, Syntax);

 public:
  ~NewExpression() final;

  static ChildNodes ArgumentsOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  NewExpression();

  DISALLOW_COPY_AND_ASSIGN(NewExpression);
};

//
// Property
//
class JOANA_AST_EXPORT Property final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(Property, Syntax);

 public:
  ~Property() final;

  static const Node& NameOf(const Node& node);
  static const Node& ValueOf(const Node& node);

 private:
  Property();

  DISALLOW_COPY_AND_ASSIGN(Property);
};

//
// ReferenceExpression
//
class JOANA_AST_EXPORT ReferenceExpression final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ReferenceExpression, Syntax);

 public:
  ~ReferenceExpression() final;

  static const Node& NameOf(const Node& node);

 private:
  ReferenceExpression();

  DISALLOW_COPY_AND_ASSIGN(ReferenceExpression);
};

//
// RegExpLiteralExpression
//
class JOANA_AST_EXPORT RegExpLiteralExpression final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpLiteralExpression, Syntax);

 public:
  ~RegExpLiteralExpression() final;

  static const Node& RegExpOf(const Node& node);
  static const Node& FlagsOf(const Node& node);

 private:
  RegExpLiteralExpression();

  DISALLOW_COPY_AND_ASSIGN(RegExpLiteralExpression);
};

//
// UnaryExpression
//
class JOANA_AST_EXPORT UnaryExpression final
    : public SyntaxTemplate<Syntax, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(UnaryExpression, Syntax);

 public:
  ~UnaryExpression() final;

  TokenKind op() const { return parameter_at<0>(); }

  static const Node& ExpressionOf(const Node& node);
  static const Node& OperatorOf(const Node& node);

 private:
  explicit UnaryExpression(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(UnaryExpression);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EXPRESSIONS_H_

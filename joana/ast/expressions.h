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

//
// Expression
//
class JOANA_AST_EXPORT Expression : public Syntax {
  DECLARE_ABSTRACT_AST_SYNTAX(Expression, Syntax);

 public:
  ~Expression() override;

 protected:
  Expression(SyntaxCode syntax_code, const Format& format);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

//
// ArrayInitializer
//
class JOANA_AST_EXPORT ArrayInitializer final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ArrayInitializer, Expression);

 public:
  ~ArrayInitializer() final;

 private:
  ArrayInitializer();

  DISALLOW_COPY_AND_ASSIGN(ArrayInitializer);
};

//
// AssignmentExpression
//
class JOANA_AST_EXPORT AssignmentExpression final
    : public SyntaxTemplate<Expression, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(AssignmentExpression, Expression);

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
    : public SyntaxTemplate<Expression, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(BinaryExpression, Expression);

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
class JOANA_AST_EXPORT CallExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(CallExpression, Expression);

 public:
  ~CallExpression() final;

  static ChildNodes ArgumentsOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  CallExpression();

  DISALLOW_COPY_AND_ASSIGN(CallExpression);
};

//
// CommaExpression
//
class JOANA_AST_EXPORT CommaExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(CommaExpression, Expression);

 public:
  ~CommaExpression() final;

 private:
  CommaExpression();

  DISALLOW_COPY_AND_ASSIGN(CommaExpression);
};

//
// ConditionalExpression
//
class JOANA_AST_EXPORT ConditionalExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ConditionalExpression, Expression);

 public:
  ~ConditionalExpression() final;

  static const ast::Node& ConditionOf(const ast::Node& node);
  static const ast::Node& FalseExpressionOf(const ast::Node& node);
  static const ast::Node& TrueExpressionOf(const ast::Node& node);

 private:
  ConditionalExpression();

  DISALLOW_COPY_AND_ASSIGN(ConditionalExpression);
};

//
// ComputedMemberExpression
//
class JOANA_AST_EXPORT ComputedMemberExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ComputedMemberExpression, Expression);

 public:
  ~ComputedMemberExpression() final;

  static const Node& ContainerOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  ComputedMemberExpression();

  DISALLOW_COPY_AND_ASSIGN(ComputedMemberExpression);
};

//
// DelimiterExpression
//
class JOANA_AST_EXPORT DelimiterExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(DelimiterExpression, Expression);

 public:
  ~DelimiterExpression() final;

 private:
  DelimiterExpression();

  DISALLOW_COPY_AND_ASSIGN(DelimiterExpression);
};

//
// ElisionExpression
//
class JOANA_AST_EXPORT ElisionExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ElisionExpression, Expression);

 public:
  ~ElisionExpression() final;

 private:
  ElisionExpression();

  DISALLOW_COPY_AND_ASSIGN(ElisionExpression);
};

//
// GroupExpression
//
class JOANA_AST_EXPORT GroupExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(GroupExpression, Expression);

 public:
  ~GroupExpression() final;

  static const ast::Node& ExpressionOf(const ast::Node& node);

 private:
  GroupExpression();

  DISALLOW_COPY_AND_ASSIGN(GroupExpression);
};

//
// MemberExpression
//
class JOANA_AST_EXPORT MemberExpression final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(MemberExpression, Expression);

 public:
  ~MemberExpression() final;

  static const Node& ContainerOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  MemberExpression();

  DISALLOW_COPY_AND_ASSIGN(MemberExpression);
};

//
// NewExpression
//
class JOANA_AST_EXPORT NewExpression final : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(NewExpression, Expression);

 public:
  ~NewExpression() final;

  static ChildNodes ArgumentsOf(const Node& node);
  static const Node& ExpressionOf(const Node& node);

 private:
  NewExpression();

  DISALLOW_COPY_AND_ASSIGN(NewExpression);
};

//
// ObjectInitializer
//
class JOANA_AST_EXPORT ObjectInitializer final
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ObjectInitializer, Expression);

 public:
  ~ObjectInitializer() final;

 private:
  ObjectInitializer();

  DISALLOW_COPY_AND_ASSIGN(ObjectInitializer);
};

//
// ParameterList
//
class JOANA_AST_EXPORT ParameterList final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ParameterList, Syntax);

 public:
  ~ParameterList() final;

 private:
  ParameterList();

  DISALLOW_COPY_AND_ASSIGN(ParameterList);
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
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(ReferenceExpression, Expression);

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
    : public SyntaxTemplate<Expression> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpLiteralExpression, Expression);

 public:
  ~RegExpLiteralExpression() final;

  static const Node& RegExpOf(const Node& node);
  static const Node& FlagsOf(const Node& node);

 private:
  RegExpLiteralExpression();

  DISALLOW_COPY_AND_ASSIGN(RegExpLiteralExpression);
};

//
// Tuple
//
class JOANA_AST_EXPORT Tuple final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(Tuple, Syntax);

 public:
  ~Tuple() final;

 private:
  Tuple();

  DISALLOW_COPY_AND_ASSIGN(Tuple);
};

//
// UnaryExpression
//
class JOANA_AST_EXPORT UnaryExpression final
    : public SyntaxTemplate<Expression, TokenKind> {
  DECLARE_CONCRETE_AST_SYNTAX(UnaryExpression, Expression);

 public:
  ~UnaryExpression() final;

  TokenKind op() const { return parameter_at<0>(); }

  static const Node& ExpressionOf(const Node& node);
  static const Node& OperatorOf(const Node& node);

 private:
  explicit UnaryExpression(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(UnaryExpression);
};

JOANA_AST_EXPORT bool IsExpression(const Node& node);
JOANA_AST_EXPORT bool IsKnownSymbol(const Node& node);
JOANA_AST_EXPORT bool IsMemberExpression(const Node& node);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EXPRESSIONS_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_FORWARD_H_
#define JOANA_PUBLIC_AST_NODE_FORWARD_H_

namespace joana {
namespace ast {

#define FOR_EACH_ABSTRACT_AST_NODE(V) \
  V(Expression)                       \
  V(ContainerNode)                    \
  V(Literal)                          \
  V(Node)                             \
  V(Statement)                        \
  V(Token)

#define FOR_EACH_AST_EXPRESSION(V) \
  V(InvalidExpression)             \
  V(LiteralExpression)             \
  V(ReferenceExpression)

#define FOR_EACH_AST_LITERAL(V) \
  V(BooleanLiteral)             \
  V(NullLiteral)                \
  V(NumericLiteral)             \
  V(StringLiteral)              \
  V(UndefinedLiteral)

#define FOR_EACH_AST_STATEMENT(V) \
  V(BlockStatement)               \
  V(BreakStatement)               \
  V(CaseClause)                   \
  V(ContinueStatement)            \
  V(DoStatement)                  \
  V(EmptyStatement)               \
  V(ExpressionStatement)          \
  V(IfStatement)                  \
  V(InvalidStatement)             \
  V(LabeledStatement)             \
  V(ThrowStatement)               \
  V(SwitchStatement)              \
  V(TryCatchStatement)            \
  V(TryFinallyStatement)          \
  V(WhileStatement)

#define FOR_EACH_AST_TOKEN(V) \
  V(Comment)                  \
  V(Invalid)                  \
  V(Punctuator)               \
  V(Name)

#define FOR_EACH_CONCRETE_AST_NODE(V) \
  FOR_EACH_AST_EXPRESSION(V)          \
  FOR_EACH_AST_LITERAL(V)             \
  FOR_EACH_AST_STATEMENT(V)           \
  FOR_EACH_AST_TOKEN(V)               \
  V(Module)

#define V(name) class name;
FOR_EACH_ABSTRACT_AST_NODE(V)
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

class EditContext;
enum class InvalidKind;
class NodeEditor;
class NodeFactory;
enum class PunctuatorKind;

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_FORWARD_H_

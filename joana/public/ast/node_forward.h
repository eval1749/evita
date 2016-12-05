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
  V(Statement)

#define FOR_EACH_AST_EXPRESSION(V) V(LiteralExpression)

#define FOR_EACH_AST_LITERAL(V) \
  V(BooleanLiteral)             \
  V(NullLiteral)                \
  V(NumericLiteral)             \
  V(StringLiteral)              \
  V(UndefinedLiteral)

#define FOR_EACH_AST_STATEMENT(V) V(ExpressionStatment)

#define FOR_EACH_CONCRETE_AST_NODE(V) \
  FOR_EACH_AST_EXPRESSION(V)          \
  FOR_EACH_AST_LITERAL(V)             \
  FOR_EACH_AST_STATEMENT(V)           \
  V(Comment)                          \
  V(Punctuator)                       \
  V(Module)                           \
  V(Name)                             \
  V(Template)

#define V(name) class name;
FOR_EACH_ABSTRACT_AST_NODE(V)
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

class NodeEditor;
class NodeFactory;

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_FORWARD_H_

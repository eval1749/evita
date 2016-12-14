// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_FORWARD_H_
#define JOANA_PUBLIC_AST_NODE_FORWARD_H_

namespace joana {
namespace ast {

#define FOR_EACH_ABSTRACT_AST_NODE(V) \
  V(Declaration)                      \
  V(Expression)                       \
  V(ContainerNode)                    \
  V(Literal)                          \
  V(Node)                             \
  V(Statement)                        \
  V(Token)

#define FOR_EACH_AST_DECLARATION(V) \
  V(ArrowFunction)                  \
  V(Function)                       \
  V(Method)

#define FOR_EACH_AST_EXPRESSION(V) \
  V(ArrayLiteralExpression)        \
  V(AssignmentExpression)          \
  V(BinaryExpression)              \
  V(CallExpression)                \
  V(CommaExpression)               \
  V(ConditionalExpression)         \
  V(DeclarationExpression)         \
  V(GroupExpression)               \
  V(ElisionExpression)             \
  V(EmptyExpression)               \
  V(InvalidExpression)             \
  V(LiteralExpression)             \
  V(MemberExpression)              \
  V(NewExpression)                 \
  V(ObjectLiteralExpression)       \
  V(PropertyExpression)            \
  V(PropertyDefinitionExpression)  \
  V(ReferenceExpression)           \
  V(UnaryExpression)

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
  V(ConstStatement)               \
  V(ContinueStatement)            \
  V(DeclarationStatement)         \
  V(DoStatement)                  \
  V(EmptyStatement)               \
  V(ExpressionStatement)          \
  V(IfStatement)                  \
  V(InvalidStatement)             \
  V(LabeledStatement)             \
  V(LetStatement)                 \
  V(ReturnStatement)              \
  V(ThrowStatement)               \
  V(SwitchStatement)              \
  V(TryCatchStatement)            \
  V(TryCatchFinallyStatement)     \
  V(TryFinallyStatement)          \
  V(VarStatement)                 \
  V(WhileStatement)

#define FOR_EACH_AST_TOKEN(V) \
  V(Comment)                  \
  V(Empty)                    \
  V(Invalid)                  \
  V(Punctuator)               \
  V(Name)

#define FOR_EACH_CONCRETE_AST_NODE(V) \
  FOR_EACH_AST_DECLARATION(V)         \
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
class ExpressionList;
enum class FunctionKind;
enum class InvalidKind;
enum class NameId;
class NodeEditor;
class NodeFactory;
enum class PunctuatorKind;

// Expression or BlockStatement
using ArrowFunctionBody = Node;

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_FORWARD_H_

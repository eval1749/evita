// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_SYNTAX_FORWARD_H_
#define JOANA_AST_SYNTAX_FORWARD_H_

namespace joana {
namespace ast {

#define FOR_EACH_AST_BINDING_ELEMENT(V) \
  V(ArrayBindingPattern)                \
  V(BindingCommaElement)                \
  V(BindingInvalidElement)              \
  V(BindingNameElement)                 \
  V(BindingProperty)                    \
  V(BindingRestElement)                 \
  V(ObjectBindingPattern)

#define FOR_EACH_AST_COMPILATION_UNIT(V) \
  V(Externs)                             \
  V(Script)                              \
  V(Module)

#define FOR_EACH_AST_DECLARATION(V) \
  V(Annotation)                     \
  V(ArrowFunction)                  \
  V(Class)                          \
  V(Function)                       \
  V(Method)

#define FOR_EACH_AST_EXPRESSION(V) \
  V(ArrayInitializer)              \
  V(AssignmentExpression)          \
  V(BinaryExpression)              \
  V(BinaryKeywordExpression)       \
  V(CallExpression)                \
  V(ComputedMemberExpression)      \
  V(CommaExpression)               \
  V(ConditionalExpression)         \
  V(DelimiterExpression)           \
  V(ElisionExpression)             \
  V(GroupExpression)               \
  V(MemberExpression)              \
  V(NewExpression)                 \
  V(ObjectInitializer)             \
  V(ParameterList)                 \
  V(Property)                      \
  V(ReferenceExpression)           \
  V(RegExpLiteralExpression)       \
  V(Tuple)                         \
  V(UnaryExpression)               \
  V(UnaryKeywordExpression)

#define FOR_EACH_AST_JSDOC(V) \
  V(JsDocDocument)            \
  V(JsDocTag)                 \
  V(JsDocText)

#define FOR_EACH_AST_LITERAL(V) \
  V(BooleanLiteral)             \
  V(NullLiteral)                \
  V(NumericLiteral)             \
  V(StringLiteral)              \
  V(UndefinedLiteral)

#define FOR_EACH_AST_REGEXP(V) \
  V(AnyCharRegExp)             \
  V(AssertionRegExp)           \
  V(CaptureRegExp)             \
  V(CharSetRegExp)             \
  V(ComplementCharSetRegExp)   \
  V(GreedyRepeatRegExp)        \
  V(InvalidRegExp)             \
  V(LazyRepeatRegExp)          \
  V(LiteralRegExp)             \
  V(LookAheadRegExp)           \
  V(LookAheadNotRegExp)        \
  V(OrRegExp)                  \
  V(SequenceRegExp)

#define FOR_EACH_AST_STATEMENT(V) \
  V(BlockStatement)               \
  V(BreakStatement)               \
  V(CaseClause)                   \
  V(ConstStatement)               \
  V(ContinueStatement)            \
  V(DoStatement)                  \
  V(EmptyStatement)               \
  V(ExpressionStatement)          \
  V(ForStatement)                 \
  V(ForInStatement)               \
  V(ForOfStatement)               \
  V(IfElseStatement)              \
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
  V(WhileStatement)               \
  V(WithStatement)

#define FOR_EACH_AST_TOKEN(V) \
  V(Comment)                  \
  V(Empty)                    \
  V(Invalid)                  \
  V(Punctuator)               \
  V(RegExpSource)             \
  V(Name)

#define FOR_EACH_AST_TYPE(V) \
  V(AnyType)                 \
  V(FunctionType)            \
  V(InvalidType)             \
  V(NullableType)            \
  V(NonNullableType)         \
  V(OptionalType)            \
  V(RecordType)              \
  V(RestType)                \
  V(TupleType)               \
  V(TypeApplication)         \
  V(TypeGroup)               \
  V(TypeName)                \
  V(TypeProperty)            \
  V(UnionType)               \
  V(UnknownType)             \
  V(VoidType)

#define FOR_EACH_AST_SYNTAX(V)     \
  FOR_EACH_AST_BINDING_ELEMENT(V)  \
  FOR_EACH_AST_COMPILATION_UNIT(V) \
  FOR_EACH_AST_DECLARATION(V)      \
  FOR_EACH_AST_EXPRESSION(V)       \
  FOR_EACH_AST_JSDOC(V)            \
  FOR_EACH_AST_LITERAL(V)          \
  FOR_EACH_AST_REGEXP(V)           \
  FOR_EACH_AST_STATEMENT(V)        \
  FOR_EACH_AST_TOKEN(V)            \
  FOR_EACH_AST_TYPE(V)

enum class FunctionKind;
enum class FunctionTypeKind;
enum class MethodKind;
enum class NameId;
enum class RegExpAssertionKind;
struct RegExpRepeat;
enum class PunctuatorKind;
enum class SyntaxCode;
class Syntax;

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_SYNTAX_FORWARD_H_

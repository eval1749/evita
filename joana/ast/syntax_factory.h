// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_SYNTAX_FACTORY_H_
#define JOANA_AST_SYNTAX_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "joana/ast/ast_export.h"
#include "joana/ast/syntax_forward.h"
#include "joana/base/float_type.h"

namespace joana {
class Zone;
namespace ast {

//
// SyntaxFactory
//
class JOANA_AST_EXPORT SyntaxFactory final {
 public:
  explicit SyntaxFactory(Zone* zone);
  ~SyntaxFactory();

#define DECLARE_FACTORY_MEMBER_0(name) const Syntax& New##name();

#define DECLARE_FACTORY_MEMBER_1(name, type1, parameter1) \
  const Syntax& New##name(type1);

#define DECLARE_FACTORY_MEMBER_2(name, type1, parameter1, type2, parameter2) \
  const Syntax& New##name(type1, type2);

  FOR_EACH_AST_BINDING_ELEMENT(DECLARE_FACTORY_MEMBER_0)
  FOR_EACH_AST_COMPILATION_UNIT(DECLARE_FACTORY_MEMBER_0)
  FOR_EACH_AST_JSDOC(DECLARE_FACTORY_MEMBER_0)
  FOR_EACH_AST_STATEMENT(DECLARE_FACTORY_MEMBER_0)

  // Declarations
  DECLARE_FACTORY_MEMBER_1(ArrowFunction, FunctionKind, kind)
  DECLARE_FACTORY_MEMBER_0(Class)
  DECLARE_FACTORY_MEMBER_1(Function, FunctionKind, kind)
  DECLARE_FACTORY_MEMBER_2(Method, MethodKind, method_kind, FunctionKind, kind)

  // Expressions
  DECLARE_FACTORY_MEMBER_0(ArgumentList)
  DECLARE_FACTORY_MEMBER_0(ArrayInitializer)
  DECLARE_FACTORY_MEMBER_1(AssignmentExpression, PunctuatorKind, kind)
  DECLARE_FACTORY_MEMBER_1(BinaryExpression, PunctuatorKind, kind)
  DECLARE_FACTORY_MEMBER_0(CallExpression)
  DECLARE_FACTORY_MEMBER_0(ComputedMemberExpression)
  DECLARE_FACTORY_MEMBER_0(CommaExpression)
  DECLARE_FACTORY_MEMBER_0(ConditionalExpression)
  DECLARE_FACTORY_MEMBER_0(DelimiterExpression)
  DECLARE_FACTORY_MEMBER_0(GroupExpression)
  DECLARE_FACTORY_MEMBER_0(ElisionExpression)
  DECLARE_FACTORY_MEMBER_0(MemberExpression)
  DECLARE_FACTORY_MEMBER_0(NewExpression)
  DECLARE_FACTORY_MEMBER_0(ObjectInitializer)
  DECLARE_FACTORY_MEMBER_0(ParameterList)
  DECLARE_FACTORY_MEMBER_0(Property)
  DECLARE_FACTORY_MEMBER_0(ReferenceExpression)
  DECLARE_FACTORY_MEMBER_0(RegExpLiteralExpression)
  DECLARE_FACTORY_MEMBER_0(Tuple)
  DECLARE_FACTORY_MEMBER_1(UnaryExpression, PunctuatorKind, kind)

  // Literals
  DECLARE_FACTORY_MEMBER_1(BooleanLiteral, bool, value)
  DECLARE_FACTORY_MEMBER_0(NullLiteral)
  DECLARE_FACTORY_MEMBER_1(NumericLiteral, float64_t, value)
  DECLARE_FACTORY_MEMBER_0(StringLiteral)
  DECLARE_FACTORY_MEMBER_0(UndefinedLiteral)

  // RegExp
  DECLARE_FACTORY_MEMBER_0(AnyCharRegExp)
  DECLARE_FACTORY_MEMBER_1(AssertionRegExp, RegExpAssertionKind, kind)
  DECLARE_FACTORY_MEMBER_0(CaptureRegExp)
  DECLARE_FACTORY_MEMBER_0(CharSetRegExp)
  DECLARE_FACTORY_MEMBER_0(ComplementCharSetRegExp)
  DECLARE_FACTORY_MEMBER_1(GreedyRepeatRegExp, RegExpRepeat, repeat)
  DECLARE_FACTORY_MEMBER_0(InvalidRegExp)
  DECLARE_FACTORY_MEMBER_1(LazyRepeatRegExp, RegExpRepeat, repeat)
  DECLARE_FACTORY_MEMBER_0(LiteralRegExp)
  DECLARE_FACTORY_MEMBER_0(LookAheadRegExp)
  DECLARE_FACTORY_MEMBER_0(LookAheadNotRegExp)
  DECLARE_FACTORY_MEMBER_0(OrRegExp)
  DECLARE_FACTORY_MEMBER_0(SequenceRegExp)

  // Tokens
  DECLARE_FACTORY_MEMBER_0(Comment)
  DECLARE_FACTORY_MEMBER_0(Empty)
  DECLARE_FACTORY_MEMBER_1(Invalid, int, error_code)
  DECLARE_FACTORY_MEMBER_0(JsDocToken)
  DECLARE_FACTORY_MEMBER_1(Punctuator, PunctuatorKind, kind)
  DECLARE_FACTORY_MEMBER_1(Name, int, number)

  // Types
  DECLARE_FACTORY_MEMBER_0(AnyType)
  DECLARE_FACTORY_MEMBER_1(FunctionType, FunctionTypeKind, kind)
  DECLARE_FACTORY_MEMBER_0(InvalidType)
  DECLARE_FACTORY_MEMBER_0(NullableType)
  DECLARE_FACTORY_MEMBER_0(NonNullableType)
  DECLARE_FACTORY_MEMBER_0(OptionalType)
  DECLARE_FACTORY_MEMBER_0(RecordType)
  DECLARE_FACTORY_MEMBER_0(RestType)
  DECLARE_FACTORY_MEMBER_0(TupleType)
  DECLARE_FACTORY_MEMBER_0(TypeApplication)
  DECLARE_FACTORY_MEMBER_0(TypeGroup)
  DECLARE_FACTORY_MEMBER_0(TypeName)
  DECLARE_FACTORY_MEMBER_0(TypeProperty)
  DECLARE_FACTORY_MEMBER_0(UnionType)
  DECLARE_FACTORY_MEMBER_0(UnknownType)
  DECLARE_FACTORY_MEMBER_0(VoidType)

#undef DECLARE_FACTORY_MEMBER_0
#undef DECLARE_FACTORY_MEMBER_1
#undef DECLARE_FACTORY_MEMBER_2

 private:
  class Cache;

  Zone& zone_;

  // |Cache| constructor takes |zone_|.
  const std::unique_ptr<Cache> cache_;

  DISALLOW_COPY_AND_ASSIGN(SyntaxFactory);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_SYNTAX_FACTORY_H_

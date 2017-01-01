// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <map>
#include <tuple>
#include <unordered_map>

#include "joana/ast/syntax_factory.h"

#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/literals.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/types.h"

namespace joana {
namespace ast {

#define FOR_EACH_PARAMETER(V)              \
  V(float64, float64_t)                    \
  V(function_kind, FunctionKind)           \
  V(function_type_kind, FunctionTypeKind)  \
  V(method_kind, MethodKind, FunctionKind) \
  V(name, int)                             \
  V(punctuator, PunctuatorKind)            \
  V(regexp_assetion, RegExpAssertionKind)  \
  V(regexp_repeat, RegExpRepeat)           \
  V(size, size_t)

//
// SyntaxFactory::Cache
//
class SyntaxFactory::Cache final {
 public:
  explicit Cache(Zone* zone);
  ~Cache() = default;

  const Syntax& BoolSyntaxOf(bool value) const;
  const Syntax& Set(const Syntax& op);
  const Syntax* TryGet(SyntaxCode opcode) const;

  // TODO(eval1749): We should share |Cache| implementation with
  // |TypeFactory::Cache| via template.
  template <typename Key>
  const Syntax* Find(const Key& key) {
    const auto& map = MapFor(key);
    const auto& it = map.find(key);
    return it == map.end() ? nullptr : it->second;
  }

  template <typename Key>
  const Syntax& Register(const Key& key, const Syntax& op) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, &op);
    DCHECK(result.second);
    return *result.first->second;
  }

 private:
  const Syntax& false_syntax_;
  const Syntax& true_syntax_;

  std::array<const Syntax*, kNumberOfOperations> syntaxes_;

#define V(name, ...)                                         \
  auto& MapFor(const std::tuple<SyntaxCode, __VA_ARGS__>&) { \
    return name##_map_;                                      \
  }                                                          \
  std::map<std::tuple<SyntaxCode, __VA_ARGS__>, const Syntax*> name##_map_;
  FOR_EACH_PARAMETER(V)
#undef V

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

SyntaxFactory::Cache::Cache(Zone* zone)
    : false_syntax_(*new (zone) BooleanLiteralSyntax(false)),
      true_syntax_(*new (zone) BooleanLiteralSyntax(true)) {
  syntaxes_.fill(nullptr);
}

const Syntax& SyntaxFactory::Cache::BoolSyntaxOf(bool data) const {
  return data ? true_syntax_ : false_syntax_;
}

const Syntax& SyntaxFactory::Cache::Set(const Syntax& op) {
  DCHECK_EQ(syntaxes_.at(static_cast<size_t>(op.opcode())), nullptr);
  DCHECK_EQ(op.format().number_of_parameters(), 0u) << op;
  syntaxes_[static_cast<size_t>(op.opcode())] = &op;
  return op;
}

const Syntax* SyntaxFactory::Cache::TryGet(SyntaxCode opcode) const {
  return syntaxes_[static_cast<size_t>(opcode)];
}

//
// SyntaxFactory
//
SyntaxFactory::SyntaxFactory(Zone* zone)
    : zone_(*zone), cache_(new Cache(zone)) {}

SyntaxFactory::~SyntaxFactory() = default;

#define IMPLEMENT_FACTORY_MEMBER_0(name)                        \
  const Syntax& SyntaxFactory::New##name() {                    \
    if (const auto* present = cache_->TryGet(SyntaxCode::name)) \
      return *present;                                          \
    return cache_->Set(*new (&zone_) name##Syntax());           \
  }

#define IMPLEMENT_FACTORY_MEMBER_1(name, type1, parameter1)          \
  const Syntax& SyntaxFactory::New##name(type1 parameter1) {         \
    const auto& key = std::make_tuple(SyntaxCode::name, parameter1); \
    if (const auto* present = cache_->Find(key))                     \
      return *present;                                               \
    const auto& new_op = *new (&zone_) name##Syntax(parameter1);     \
    return cache_->Register(key, new_op);                            \
  }

#define IMPLEMENT_FACTORY_MEMBER_2(name, type1, parameter1, type2, parameter2) \
  const Syntax& SyntaxFactory::New##name(type1 parameter1, type2 parameter2) { \
    const auto& key =                                                          \
        std::make_tuple(SyntaxCode::name, parameter1, parameter2);             \
    if (const auto* present = cache_->Find(key))                               \
      return *present;                                                         \
    const auto& new_op = *new (&zone_) name##Syntax(parameter1, parameter2);   \
    return cache_->Register(key, new_op);                                      \
  }

FOR_EACH_AST_BINDING_ELEMENT(IMPLEMENT_FACTORY_MEMBER_0)
FOR_EACH_AST_COMPILATION_UNIT(IMPLEMENT_FACTORY_MEMBER_0)

// Declarations
IMPLEMENT_FACTORY_MEMBER_1(ArrowFunction, FunctionKind, kind)
IMPLEMENT_FACTORY_MEMBER_0(Class)
IMPLEMENT_FACTORY_MEMBER_1(Function, FunctionKind, kind)
IMPLEMENT_FACTORY_MEMBER_2(Method, MethodKind, method_kind, FunctionKind, kind)

// Expressions
IMPLEMENT_FACTORY_MEMBER_0(ArgumentList)
IMPLEMENT_FACTORY_MEMBER_0(ArrayInitializer)
IMPLEMENT_FACTORY_MEMBER_1(AssignmentExpression, PunctuatorKind, kind)
IMPLEMENT_FACTORY_MEMBER_1(BinaryExpression, PunctuatorKind, kind)
IMPLEMENT_FACTORY_MEMBER_0(CallExpression)
IMPLEMENT_FACTORY_MEMBER_0(ComputedMemberExpression)
IMPLEMENT_FACTORY_MEMBER_0(CommaExpression)
IMPLEMENT_FACTORY_MEMBER_0(ConditionalExpression)
IMPLEMENT_FACTORY_MEMBER_0(DelimiterExpression)
IMPLEMENT_FACTORY_MEMBER_0(GroupExpression)
IMPLEMENT_FACTORY_MEMBER_0(ElisionExpression)
IMPLEMENT_FACTORY_MEMBER_0(MemberExpression)
IMPLEMENT_FACTORY_MEMBER_0(NewExpression)
IMPLEMENT_FACTORY_MEMBER_0(ObjectInitializer)
IMPLEMENT_FACTORY_MEMBER_0(ParameterList)
IMPLEMENT_FACTORY_MEMBER_0(Property)
IMPLEMENT_FACTORY_MEMBER_0(ReferenceExpression)
IMPLEMENT_FACTORY_MEMBER_0(RegExpLiteralExpression)
IMPLEMENT_FACTORY_MEMBER_0(Tuple)
IMPLEMENT_FACTORY_MEMBER_1(UnaryExpression, PunctuatorKind, kind)

FOR_EACH_AST_JSDOC(IMPLEMENT_FACTORY_MEMBER_0)

// Literals
const Syntax& SyntaxFactory::NewBooleanLiteral(bool value) {
  return cache_->BoolSyntaxOf(value);
}

IMPLEMENT_FACTORY_MEMBER_0(NullLiteral)
IMPLEMENT_FACTORY_MEMBER_1(NumericLiteral, float64_t, value)
IMPLEMENT_FACTORY_MEMBER_0(StringLiteral)
IMPLEMENT_FACTORY_MEMBER_0(UndefinedLiteral)

// RegExp
IMPLEMENT_FACTORY_MEMBER_0(AnyCharRegExp)
IMPLEMENT_FACTORY_MEMBER_1(AssertionRegExp, RegExpAssertionKind, kind)
IMPLEMENT_FACTORY_MEMBER_0(CaptureRegExp)
IMPLEMENT_FACTORY_MEMBER_0(CharSetRegExp)
IMPLEMENT_FACTORY_MEMBER_0(ComplementCharSetRegExp)
IMPLEMENT_FACTORY_MEMBER_1(GreedyRepeatRegExp, RegExpRepeat, repeat)
IMPLEMENT_FACTORY_MEMBER_0(InvalidRegExp)
IMPLEMENT_FACTORY_MEMBER_1(LazyRepeatRegExp, RegExpRepeat, repeat)
IMPLEMENT_FACTORY_MEMBER_0(LiteralRegExp)
IMPLEMENT_FACTORY_MEMBER_0(LookAheadRegExp)
IMPLEMENT_FACTORY_MEMBER_0(LookAheadNotRegExp)
IMPLEMENT_FACTORY_MEMBER_0(OrRegExp)
IMPLEMENT_FACTORY_MEMBER_0(SequenceRegExp)

FOR_EACH_AST_STATEMENT(IMPLEMENT_FACTORY_MEMBER_0)

// Tokens
IMPLEMENT_FACTORY_MEMBER_0(Comment)
IMPLEMENT_FACTORY_MEMBER_0(Empty)
IMPLEMENT_FACTORY_MEMBER_1(Invalid, int, error_code)
IMPLEMENT_FACTORY_MEMBER_1(Punctuator, PunctuatorKind, kind)
IMPLEMENT_FACTORY_MEMBER_1(Name, int, number)

// Types
IMPLEMENT_FACTORY_MEMBER_0(AnyType)
IMPLEMENT_FACTORY_MEMBER_1(FunctionType, FunctionTypeKind, kind)
IMPLEMENT_FACTORY_MEMBER_0(InvalidType)
IMPLEMENT_FACTORY_MEMBER_0(NullableType)
IMPLEMENT_FACTORY_MEMBER_0(NonNullableType)
IMPLEMENT_FACTORY_MEMBER_0(OptionalType)
IMPLEMENT_FACTORY_MEMBER_0(RecordType)
IMPLEMENT_FACTORY_MEMBER_0(RestType)
IMPLEMENT_FACTORY_MEMBER_0(TupleType)
IMPLEMENT_FACTORY_MEMBER_0(TypeApplication)
IMPLEMENT_FACTORY_MEMBER_0(TypeGroup)
IMPLEMENT_FACTORY_MEMBER_0(TypeName)
IMPLEMENT_FACTORY_MEMBER_0(TypeProperty)
IMPLEMENT_FACTORY_MEMBER_0(UnionType)
IMPLEMENT_FACTORY_MEMBER_0(UnknownType)
IMPLEMENT_FACTORY_MEMBER_0(VoidType)

}  // namespace ast
}  // namespace joana

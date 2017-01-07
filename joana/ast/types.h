// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_TYPES_H_
#define JOANA_AST_TYPES_H_

#include <iosfwd>
#include <utility>
#include <vector>

#include "joana/ast/node.h"
#include "joana/ast/syntax.h"
#include "joana/ast/syntax_forward.h"
#include "joana/base/iterator_utils.h"

namespace joana {
namespace ast {

//
// FunctionTypeKind
//
#define FOR_EACH_AST_FUNCTION_TYPE_KIND(V) \
  V(Normal)                                \
  V(This)                                  \
  V(New)

enum class FunctionTypeKind {
#define V(name) name,
  FOR_EACH_AST_FUNCTION_TYPE_KIND(V)
#undef V
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          FunctionTypeKind kind);

DECLARE_AST_SYNTAX_0(AnyType)
DECLARE_AST_SYNTAX_1(FunctionType, FunctionTypeKind, kind)
DECLARE_AST_SYNTAX_0(InvalidType)
DECLARE_AST_SYNTAX_0(NullableType)
DECLARE_AST_SYNTAX_0(NonNullableType)
DECLARE_AST_SYNTAX_0(OptionalType)
DECLARE_AST_SYNTAX_0(RecordType)
DECLARE_AST_SYNTAX_0(RestType)
DECLARE_AST_SYNTAX_0(TupleType)
DECLARE_AST_SYNTAX_0(TypeGroup)
DECLARE_AST_SYNTAX_0(UnionType)
DECLARE_AST_SYNTAX_0(UnknownType)
DECLARE_AST_SYNTAX_0(VoidType)

//
// MemberType
//
class JOANA_AST_EXPORT MemberType final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(MemberType, Syntax);

 public:
  ~MemberType() final;

  static const Node& MemberOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  MemberType();

  DISALLOW_COPY_AND_ASSIGN(MemberType);
};

//
// PrimitiveType is a placeholder of primitive type. The parser does not use
// this node.
//
class JOANA_AST_EXPORT PrimitiveType final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(PrimitiveType, Syntax);

 public:
  ~PrimitiveType() final;

  static const Node& NameOf(const Node& node);

 private:
  PrimitiveType();

  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

//
// TypeApplication
//
class JOANA_AST_EXPORT TypeApplication final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(TypeApplication, Syntax);

 public:
  ~TypeApplication() final;

  static const Node& ArgumentsOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  TypeApplication();

  DISALLOW_COPY_AND_ASSIGN(TypeApplication);
};

//
// TypeName
//
class JOANA_AST_EXPORT TypeName final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(TypeName, Syntax);

 public:
  ~TypeName() final;

  static const Node& NameOf(const Node& node);

 private:
  TypeName();

  DISALLOW_COPY_AND_ASSIGN(TypeName);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TYPES_H_

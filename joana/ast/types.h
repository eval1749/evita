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
DECLARE_AST_SYNTAX_0(TypeName)
DECLARE_AST_SYNTAX_0(TypeProperty)
DECLARE_AST_SYNTAX_0(UnionType)
DECLARE_AST_SYNTAX_0(UnknownType)
DECLARE_AST_SYNTAX_0(VoidType)

//
// TypeApplicationSyntax
//
class JOANA_AST_EXPORT TypeApplicationSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(TypeApplication, Syntax);

 public:
  ~TypeApplicationSyntax() final;

  static const Node& ArgumentsOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  TypeApplicationSyntax();

  DISALLOW_COPY_AND_ASSIGN(TypeApplicationSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TYPES_H_

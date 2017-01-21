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

//
// Type
//
class JOANA_AST_EXPORT Type : public Syntax {
  DECLARE_ABSTRACT_AST_SYNTAX(Type, Syntax);

 public:
  ~Type() override;

 protected:
  Type(SyntaxCode syntax_code, const Format& format);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

//
// AnyType
//
class JOANA_AST_EXPORT AnyType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(AnyType, Type);

 public:
  ~AnyType() final;

 private:
  AnyType();

  DISALLOW_COPY_AND_ASSIGN(AnyType);
};

//
// FunctionType
//
class JOANA_AST_EXPORT FunctionType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(FunctionType, Type);

 public:
  ~FunctionType() final;

  FunctionTypeKind kind() const { return kind_; }

 private:
  explicit FunctionType(FunctionTypeKind kind);

  const FunctionTypeKind kind_;

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//
// InvalidType
//
class JOANA_AST_EXPORT InvalidType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(InvalidType, Type);

 public:
  ~InvalidType() final;

 private:
  InvalidType();

  DISALLOW_COPY_AND_ASSIGN(InvalidType);
};

//
// MemberType
//
class JOANA_AST_EXPORT MemberType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(MemberType, Type);

 public:
  ~MemberType() final;

  static const Node& MemberOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  MemberType();

  DISALLOW_COPY_AND_ASSIGN(MemberType);
};

//
// NullableType
//
class JOANA_AST_EXPORT NullableType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(NullableType, Type);

 public:
  ~NullableType() final;

  static const Node& TypeOf(const Node& node);

 private:
  NullableType();

  DISALLOW_COPY_AND_ASSIGN(NullableType);
};

//
// NonNullableType
//
class JOANA_AST_EXPORT NonNullableType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(NonNullableType, Type);

 public:
  ~NonNullableType() final;

  static const Node& TypeOf(const Node& node);

 private:
  NonNullableType();

  DISALLOW_COPY_AND_ASSIGN(NonNullableType);
};

//
// OptionalType
//
class JOANA_AST_EXPORT OptionalType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(OptionalType, Type);

 public:
  ~OptionalType() final;

  static const Node& TypeOf(const Node& node);

 private:
  OptionalType();

  DISALLOW_COPY_AND_ASSIGN(OptionalType);
};

//
// PrimitiveType is a placeholder of primitive type. The parser does not use
// this node.
//
class JOANA_AST_EXPORT PrimitiveType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(PrimitiveType, Type);

 public:
  ~PrimitiveType() final;

  static const Node& NameOf(const Node& node);

 private:
  PrimitiveType();

  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

//
// RecordType
//
class JOANA_AST_EXPORT RecordType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(RecordType, Type);

 public:
  ~RecordType() final;

  static const Node& TypeOf(const Node& node);

 private:
  RecordType();

  DISALLOW_COPY_AND_ASSIGN(RecordType);
};

//
// RestType
//
class JOANA_AST_EXPORT RestType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(RestType, Type);

 public:
  ~RestType() final;

  static const Node& TypeOf(const Node& node);

 private:
  RestType();

  DISALLOW_COPY_AND_ASSIGN(RestType);
};

//
// TupleType
//
class JOANA_AST_EXPORT TupleType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(TupleType, Type);

 public:
  ~TupleType() final;

  static const Node& TypeOf(const Node& node);

 private:
  TupleType();

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

//
// TypeApplication
//
class JOANA_AST_EXPORT TypeApplication final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(TypeApplication, Type);

 public:
  ~TypeApplication() final;

  static const Node& ArgumentsOf(const Node& node);
  static const Node& NameOf(const Node& node);

 private:
  TypeApplication();

  DISALLOW_COPY_AND_ASSIGN(TypeApplication);
};

//
// TypeGroup
//
class JOANA_AST_EXPORT TypeGroup final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(TypeGroup, Type);

 public:
  ~TypeGroup() final;

  static const Node& TypeOf(const Node& node);

 private:
  TypeGroup();

  DISALLOW_COPY_AND_ASSIGN(TypeGroup);
};

//
// TypeName
//
class JOANA_AST_EXPORT TypeName final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(TypeName, Type);

 public:
  ~TypeName() final;

  static const Node& NameOf(const Node& node);

 private:
  TypeName();

  DISALLOW_COPY_AND_ASSIGN(TypeName);
};

//
// UnionType
//
class JOANA_AST_EXPORT UnionType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(UnionType, Type);

 public:
  ~UnionType() final;

 private:
  UnionType();

  DISALLOW_COPY_AND_ASSIGN(UnionType);
};

//
// UnknownType
//
class JOANA_AST_EXPORT UnknownType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(UnknownType, Type);

 public:
  ~UnknownType() final;

 private:
  UnknownType();

  DISALLOW_COPY_AND_ASSIGN(UnknownType);
};

//
// VoidType
//
class JOANA_AST_EXPORT VoidType final : public Type {
  DECLARE_CONCRETE_AST_SYNTAX(VoidType, Type);

 public:
  ~VoidType() final;

 private:
  VoidType();

  DISALLOW_COPY_AND_ASSIGN(VoidType);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TYPES_H_

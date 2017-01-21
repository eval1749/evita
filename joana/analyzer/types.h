// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPES_H_
#define JOANA_ANALYZER_TYPES_H_

#include <set>
#include <utility>
#include <vector>

#include "joana/analyzer/type.h"

#include "joana/base/block_range.h"
#include "joana/base/iterator_utils.h"
#include "joana/base/memory/zone_unordered_map.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {

class Zone;

namespace analyzer {

class Class;
class TypeParameter;
class Value;

// See "type_forward.h" for list of types.

enum class FunctionTypeKind {
  Constructor,
  Normal,
};

//
// NamedType
//
class NamedType : public Type {
  DECLARE_ABSTRACT_ANALYZE_TYPE(NamedType, Type)

 public:
  ~NamedType();

  bool is_anonymous() const;
  const ast::Node& name() const { return name_; }

 protected:
  NamedType(int id, const ast::Node& name);

 private:
  const ast::Node& name_;

  DISALLOW_COPY_AND_ASSIGN(NamedType);
};

//
// AnyType
//
class AnyType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(AnyType, Type)

 public:
  ~AnyType() final;

 private:
  explicit AnyType(int id);

  DISALLOW_COPY_AND_ASSIGN(AnyType);
};

//
// ClassType
//
class ClassType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(ClassType, Type)

 public:
  ~ClassType() final;

  const ast::Node& name() const;
  Value& value() const { return value_; }

 private:
  ClassType(int id, Value* value);

  Value& value_;

  DISALLOW_COPY_AND_ASSIGN(ClassType);
};

//
// FunctionType
//
class FunctionType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(FunctionType, Type)

 public:
  void* operator new(size_t size, void* pointer) { return pointer; }

  ~FunctionType() final;

  FunctionTypeKind kind() const { return kind_; }
  BlockRange<const Type*> parameters() const;
  BlockRange<const TypeParameter*> type_parameters() const;
  const Type& return_type() const { return return_type_; }
  const Type& this_type() const { return this_type_; }

 private:
  FunctionType(int id,
               FunctionTypeKind kind,
               const std::vector<const TypeParameter*>& type_parameters,
               const std::vector<const Type*>& parameters,
               const Type& return_type,
               const Type& this_type);

  const FunctionTypeKind kind_;
  const size_t number_of_parameters_;
  const size_t number_of_type_parameters_;
  const Type& return_type_;
  const Type& this_type_;
  const Type* elements_[1];

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//
// InvalidType
//
class InvalidType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(InvalidType, Type)

 public:
  ~InvalidType() final;

 private:
  explicit InvalidType(int id);

  DISALLOW_COPY_AND_ASSIGN(InvalidType);
};

//
// NilType
//
class NilType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(NilType, Type)

 public:
  ~NilType() final;

 private:
  explicit NilType(int id);

  DISALLOW_COPY_AND_ASSIGN(NilType);
};

//
// NullType
//
class NullType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(NullType, Type)

 public:
  ~NullType() final;

 private:
  explicit NullType(int id);

  DISALLOW_COPY_AND_ASSIGN(NullType);
};

//
// PrimitiveType
//
class PrimitiveType final : public NamedType {
  DECLARE_CONCRETE_ANALYZE_TYPE(PrimitiveType, NamedType)

 public:
  ~PrimitiveType() final;

 private:
  PrimitiveType(int id, const ast::Node& name);

  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

//
// TupleType
//
class TupleType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(TupleType, Type)

 public:
  void* operator new(size_t size, void* pointer) { return pointer; }

  ~TupleType() final;

  BlockRange<const Type*> members() const;

 private:
  TupleType(int id, const std::vector<const Type*>& members);

  const size_t number_of_members_;
  const Type* members_[1];

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

//
// TypeAlias
//
class TypeAlias final : public NamedType {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeAlias, NamedType)

 public:
  ~TypeAlias() final;

 private:
  TypeAlias(int id, const ast::Node& name, const ast::Node& type);

  const ast::Node& type_;

  DISALLOW_COPY_AND_ASSIGN(TypeAlias);
};

//
// TypeName
//
class TypeName final : public NamedType {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeName, NamedType)

 public:
  ~TypeName() final;

 private:
  TypeName(int id, const ast::Node& name);

  DISALLOW_COPY_AND_ASSIGN(TypeName);
};

//
// TypeParameter
//
class TypeParameter final : public NamedType {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeParameter, Type)

 public:
  ~TypeParameter() final;

 private:
  TypeParameter(int id, const ast::Node& name);

  DISALLOW_COPY_AND_ASSIGN(TypeParameter);
};

//
// UnionType
//
class UnionType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(UnionType, Type)

 public:
  void* operator new(size_t size, void* pointer) { return pointer; }

  ~UnionType() final;

  BlockRange<const Type*> members() const;

 private:
  UnionType(int id, const std::set<const Type*>& members);

  const size_t number_of_members_;
  const Type* members_[1];

  DISALLOW_COPY_AND_ASSIGN(UnionType);
};

//
// UnspecifiedType
//
class UnspecifiedType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(UnspecifiedType, Type)

 public:
  ~UnspecifiedType() final;

 private:
  explicit UnspecifiedType(int id);

  DISALLOW_COPY_AND_ASSIGN(UnspecifiedType);
};

//
// VoidType
//
class VoidType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(VoidType, Type)

 public:
  ~VoidType() final;

 private:
  explicit VoidType(int id);

  DISALLOW_COPY_AND_ASSIGN(VoidType);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPES_H_

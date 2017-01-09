// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPES_H_
#define JOANA_ANALYZER_TYPES_H_

#include <utility>
#include <vector>

#include "joana/analyzer/type.h"

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
// GenericType
//
class GenericType : public NamedType {
  DECLARE_ABSTRACT_ANALYZE_TYPE(GenericType, NamedType)

 public:
  ~GenericType() override;

  bool has_parameters() const { return !parameters_.empty(); }
  auto parameters() const { return ReferenceRangeOf(parameters_); }

 protected:
  // |name| is |ast::Name|, |ast::ComputedMemberExpression| or
  // |ast::MemberExpression|.
  GenericType(Zone* zone,
              int id,
              const ast::Node& name,
              const std::vector<const TypeParameter*>& parameters);

 private:
  const ZoneVector<const TypeParameter*> parameters_;

  DISALLOW_COPY_AND_ASSIGN(GenericType);
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
class ClassType final : public GenericType {
  DECLARE_CONCRETE_ANALYZE_TYPE(ClassType, GenericType)

 public:
  ~ClassType() final;

  Class& value() const { return *value_; }

 private:
  ClassType(Zone* zone,
            int id,
            const ast::Node& name,
            const std::vector<const TypeParameter*>& parameters,
            Class* value);

  Class* const value_;

  DISALLOW_COPY_AND_ASSIGN(ClassType);
};

//
// FunctionType
//
class FunctionType final : public GenericType {
  DECLARE_CONCRETE_ANALYZE_TYPE(FunctionType, GenericType)

 public:
  ~FunctionType() final;

  FunctionTypeKind kind() const { return kind_; }
  auto parameter_types() const { return ReferenceRangeOf(parameter_types_); }
  const Type& return_type() const { return return_type_; }
  const Type& this_type() const { return this_type_; }

 private:
  FunctionType(Zone* zone,
               int id,
               FunctionTypeKind kind,
               const ast::Node& name,
               const std::vector<const TypeParameter*>& parameters,
               const std::vector<const Type*> parameter_types,
               const Type& return_type,
               const Type& this_type);

  const FunctionTypeKind kind_;
  const ZoneVector<const Type*> parameter_types_;
  const Type& return_type_;
  const Type& this_type_;

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
// TypeApplication
//
class TypeApplication final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeApplication, Type)

 public:
  using Argument = std::pair<const TypeParameter*, const Type*>;

  ~TypeApplication() final;

  const ZoneVector<Argument>& arguments() const { return arguments_; }
  const GenericType& generic_type() const { return generic_type_; }

 private:
  TypeApplication(Zone* zone,
                  int id,
                  const GenericType& generic_type,
                  const std::vector<Argument>& arguments);

  const ZoneVector<Argument> arguments_;
  const GenericType& generic_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeApplication);
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
// UnknownType
//
class UnknownType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(UnknownType, Type)

 public:
  ~UnknownType() final;

 private:
  explicit UnknownType(int id);

  DISALLOW_COPY_AND_ASSIGN(UnknownType);
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

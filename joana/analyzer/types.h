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

class TypeParameter;
class Variable;

//
// GenericType
//
class GenericType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(GenericType, Type)

 public:
  ~GenericType() final;

  auto parameters() const { return ReferenceRangeOf(parameters_); }

 private:
  GenericType(Zone* zone,
              int id,
              const ast::Node& node,
              const std::vector<const TypeParameter*>& parameters);

  const ZoneVector<const TypeParameter*> parameters_;

  DISALLOW_COPY_AND_ASSIGN(GenericType);
};

//
// PrimitiveType
//
class PrimitiveType final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(PrimitiveType, Type)

 public:
  ~PrimitiveType() final;

 private:
  PrimitiveType(int id, const ast::Node& node);

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
                  const ast::Node& node,
                  const GenericType& generic_type,
                  const std::vector<Argument>& arguments);

  const ZoneVector<Argument> arguments_;
  const GenericType& generic_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeApplication);
};

//
// TypeName
//
class TypeName final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeName, Type)

 public:
  ~TypeName() final;

 private:
  TypeName(int id, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(TypeName);
};

//
// TypeParameter
//
class TypeParameter final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeParameter, Type)

 public:
  ~TypeParameter() final;

 private:
  TypeParameter(int id, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(TypeParameter);
};

//
// TypeReference
//
class TypeReference final : public Type {
  DECLARE_CONCRETE_ANALYZE_TYPE(TypeReference, Type)

 public:
  ~TypeReference() final;

  Variable& variable() const { return variable_; }

 private:
  TypeReference(int id, Variable* variable);

  Variable& variable_;

  DISALLOW_COPY_AND_ASSIGN(TypeReference);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPES_H_

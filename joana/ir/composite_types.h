// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <type_traits>
#include <vector>

#ifndef JOANA_IR_COMPOSITE_TYPES_H_
#define JOANA_IR_COMPOSITE_TYPES_H_

#include <tuple>

#include "joana/base/iterator_utils.h"
#include "joana/base/memory/zone_vector.h"
#include "joana/ir/type.h"

namespace joana {
namespace ir {

//
// CompositeType
//
class JOANA_IR_EXPORT CompositeType : public Type {
  DECLARE_ABSTRACT_IR_TYPE(CompositeType, Type);

 public:
  ~CompositeType();

 protected:
  CompositeType();

 private:
  DISALLOW_COPY_AND_ASSIGN(CompositeType);
};

//
// CompositeTypeTemplate
//
template <typename Base, typename... Members>
class CompositeTypeTemplate : public Base {
  static_assert(sizeof...(Members) >= 1,
                "CompositeTypeTemplate should have at least one member.");

 public:
  ~CompositeTypeTemplate() = default;

 protected:
  template <typename... Parameters>
  CompositeTypeTemplate(const std::tuple<Members...> members,
                        Parameters... parameters)
      : Base(parameters...), members_(members) {}

  template <size_t kIndex>
  auto member_at() const {
    return std::get<kIndex>(members_);
  }

 private:
  std::tuple<Members...> members_;

  DISALLOW_COPY_AND_ASSIGN(CompositeTypeTemplate);
};

//
// FunctionType
//
class JOANA_IR_EXPORT FunctionType
    : public CompositeTypeTemplate<CompositeType,
                                   const TupleType*,
                                   const Type*> {
  DECLARE_CONCRETE_IR_TYPE(FunctionType, CompositeType);

 public:
  ~FunctionType();

  const TupleType& parameters_type() const { return *member_at<0>(); }
  const Type& return_type() const { return *member_at<1>(); }

 private:
  friend class CompositeTypeFactory;

  FunctionType(const TupleType& parameters_type, const Type& return_type);

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//
// ReferenceType
//
class JOANA_IR_EXPORT ReferenceType
    : public CompositeTypeTemplate<CompositeType, const Type*> {
  DECLARE_CONCRETE_IR_TYPE(ReferenceType, CompositeType);

 public:
  ~ReferenceType();

  const Type& to() const { return *member_at<0>(); }

 private:
  friend class CompositeTypeFactory;

  explicit ReferenceType(const Type& return_type);

  DISALLOW_COPY_AND_ASSIGN(ReferenceType);
};

//
// TupleType
//
class JOANA_IR_EXPORT TupleType : public CompositeType {
  DECLARE_CONCRETE_IR_TYPE(TupleType, CompositeType);

 public:
  ~TupleType();

  auto members() const { return ReferenceRangeOf(members_); }

 private:
  friend class CompositeTypeFactory;

  TupleType(Zone* zone, const std::vector<const Type*>& members);

  ZoneVector<const Type*> members_;

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_COMPOSITE_TYPES_H_

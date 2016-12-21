// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/ir/composite_types.h"

namespace joana {
namespace ir {

//
// CompositeType
//
CompositeType::CompositeType() = default;
CompositeType::~CompositeType() = default;

//
// FunctionType
//
FunctionType::FunctionType(const TupleType& parameters_type,
                           const Type& return_type)
    : CompositeTypeTemplate(std::make_tuple(&parameters_type, &return_type)) {}

FunctionType::~FunctionType() = default;

//
// ReferenceType
//
ReferenceType::ReferenceType(const Type& to) : CompositeTypeTemplate(&to) {}

ReferenceType::~ReferenceType() = default;

//
// TupleType
//
TupleType::TupleType(Zone* zone, const std::vector<const Type*>& members)
    : members_(zone, members) {}

TupleType::~TupleType() = default;

}  // namespace ir
}  // namespace joana

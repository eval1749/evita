// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/types.h"

namespace joana {
namespace ast {

//
// AnyType
//
AnyType::AnyType(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

AnyType::~AnyType() = default;

//
// FunctionType
//
FunctionType::FunctionType(const SourceCodeRange& range,
                           FunctionTypeKind kind,
                           const TypeList& parameter_types,
                           const Type& return_type)
    : NodeTemplate(std::make_tuple(kind, &parameter_types, &return_type),
                   range) {}

FunctionType::~FunctionType() = default;

//
// InvalidType
//
InvalidType::InvalidType(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

InvalidType::~InvalidType() = default;

//
// NullableType
//
NullableType::NullableType(const SourceCodeRange& range, const Type& type)
    : NodeTemplate(std::make_tuple(&type), range) {}

NullableType::~NullableType() = default;

//
// NonNullableType
//
NonNullableType::NonNullableType(const SourceCodeRange& range, const Type& type)
    : NodeTemplate(std::make_tuple(&type), range) {}

NonNullableType::~NonNullableType() = default;

//
// OptionalType
//
OptionalType::OptionalType(const SourceCodeRange& range, const Type& type)
    : NodeTemplate(std::make_tuple(&type), range) {}

OptionalType::~OptionalType() = default;

//
// RecordType
//
RecordType::RecordType(const SourceCodeRange& range,
                       const RecordTypeMembers& members)
    : NodeTemplate(std::make_tuple(&members), range) {}

RecordType::~RecordType() = default;

//
// RecordTypeMembers
//
RecordTypeMembers::RecordTypeMembers(Zone* zone,
                                     const std::vector<Member>& members)
    : members_(zone, members) {}

RecordTypeMembers::~RecordTypeMembers() = default;

//
// RestType
//
RestType::RestType(const SourceCodeRange& range, const Type& type)
    : NodeTemplate(std::make_tuple(&type), range) {}

RestType::~RestType() = default;

//
// TupleType
//
TupleType::TupleType(const SourceCodeRange& range, const TypeList& members)
    : NodeTemplate(std::make_tuple(&members), range) {}

TupleType::~TupleType() = default;

//
// Type
//
Type::Type(const SourceCodeRange& range) : Node(range) {}
Type::~Type() = default;

//
// TypeApplication
//
TypeApplication::TypeApplication(const SourceCodeRange& range,
                                 const Name& name,
                                 const TypeList& members)
    : NodeTemplate(std::make_tuple(&name, &members), range) {}

TypeApplication::~TypeApplication() = default;

//
// TypeList
//
TypeList::TypeList(Zone* zone, const std::vector<const Type*>& types)
    : types_(zone, types) {}

TypeList::~TypeList() = default;

//
// TypeName
//
TypeName::TypeName(const SourceCodeRange& range, const Name& name)
    : NodeTemplate(std::make_tuple(&name), range) {}

TypeName::~TypeName() = default;

//
// UnionType
//
UnionType::UnionType(const SourceCodeRange& range, const TypeList& members)
    : NodeTemplate(std::make_tuple(&members), range) {}

UnionType::~UnionType() = default;

//
// UnknownType
//
UnknownType::UnknownType(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

UnknownType::~UnknownType() = default;

//
// VoidType
//
VoidType::VoidType(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

VoidType::~VoidType() = default;

}  // namespace ast
}  // namespace joana

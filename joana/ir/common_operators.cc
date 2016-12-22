// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ir/common_operators.h"

namespace joana {
namespace ir {

//
// ExitOperator
//
ExitOperator::ExitOperator()
    : OperatorTemplate(std::tuple<>(),
                       OperationCode::Exit,
                       Format::Builder()
                           .set_number_of_inputs(1)
                           .set_number_of_members(0)
                           .Build()) {}

ExitOperator::~ExitOperator() = default;

//
// LiteralBoolOperator
//
LiteralBoolOperator::LiteralBoolOperator(bool data)
    : OperatorTemplate(data,
                       OperationCode::LiteralBool,
                       Format::Builder().set_number_of_members(1).Build()) {}

LiteralBoolOperator::~LiteralBoolOperator() = default;

//
// LiteralFloat64Operator
//
LiteralFloat64Operator::LiteralFloat64Operator(float64_t data)
    : OperatorTemplate(data,
                       OperationCode::LiteralFloat64,
                       Format::Builder().set_number_of_members(1).Build()) {}

LiteralFloat64Operator::~LiteralFloat64Operator() = default;

//
// LiteralInt64Operator
//
LiteralInt64Operator::LiteralInt64Operator(int64_t data)
    : OperatorTemplate(data,
                       OperationCode::LiteralInt64,
                       Format::Builder().set_number_of_members(1).Build()) {}

LiteralInt64Operator::~LiteralInt64Operator() = default;

//
// LiteralStringOperator
//
LiteralStringOperator::LiteralStringOperator(base::StringPiece16 data)
    : OperatorTemplate(data,
                       OperationCode::LiteralString,
                       Format::Builder().set_number_of_members(1).Build()) {}

LiteralStringOperator::~LiteralStringOperator() = default;

//
// LiteralVoidOperator
//
LiteralVoidOperator::LiteralVoidOperator()
    : OperatorTemplate(std::tuple<>(),
                       OperationCode::LiteralVoid,
                       Format::Builder().set_number_of_members(0).Build()) {}

LiteralVoidOperator::~LiteralVoidOperator() = default;

//
// ProjectionOperator
//
ProjectionOperator::ProjectionOperator(size_t index)
    : OperatorTemplate(index,
                       OperationCode::Projection,
                       Format::Builder()
                           .set_number_of_inputs(1)
                           .set_number_of_members(1)
                           .Build()) {}

ProjectionOperator::~ProjectionOperator() = default;

//
// RetOperator
//
RetOperator::RetOperator()
    : OperatorTemplate(std::tuple<>(),
                       OperationCode::Ret,
                       Format::Builder()
                           .set_number_of_inputs(2)
                           .set_number_of_members(0)
                           .Build()) {}

RetOperator::~RetOperator() = default;

//
// StartOperator
//
StartOperator::StartOperator()
    : OperatorTemplate(std::tuple<>(),
                       OperationCode::Start,
                       Format::Builder().set_number_of_members(0).Build()) {}

StartOperator::~StartOperator() = default;

//
// TupleOperator
//
TupleOperator::TupleOperator(size_t size)
    : OperatorTemplate(size,
                       OperationCode::Tuple,
                       Format::Builder()
                           .set_number_of_inputs(size)
                           .set_number_of_members(1)
                           .Build()) {}

TupleOperator::~TupleOperator() = default;

}  // namespace ir
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/literals.h"

#include "joana/base/escaped_string_piece.h"

namespace joana {
namespace ast {

//
// BooleanLiteral
//
BooleanLiteral::BooleanLiteral(bool value)
    : SyntaxTemplate(value,
                     SyntaxCode::BooleanLiteral,
                     Format::Builder()
                         .set_is_literal(true)
                         .set_number_of_parameters(1)
                         .Build()) {}

BooleanLiteral::~BooleanLiteral() = default;

//
// NumericLiteral
//
NumericLiteral::NumericLiteral(float64_t value)
    : SyntaxTemplate(value,
                     SyntaxCode::NumericLiteral,
                     Format::Builder()
                         .set_is_literal(true)
                         .set_number_of_parameters(1)
                         .Build()) {}

NumericLiteral::~NumericLiteral() = default;

//
// NullLiteral
//
NullLiteral::NullLiteral()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::NullLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

NullLiteral::~NullLiteral() = default;

//
// StringLiteral
//
StringLiteral::StringLiteral()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::StringLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

StringLiteral::~StringLiteral() = default;

//
// UndefinedLiteral
//
UndefinedLiteral::UndefinedLiteral()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::UndefinedLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

UndefinedLiteral::~UndefinedLiteral() = default;

}  // namespace ast
}  // namespace joana

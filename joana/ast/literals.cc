// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/literals.h"

#include "joana/base/escaped_string_piece.h"

namespace joana {
namespace ast {

//
// BooleanLiteralSyntax
//
BooleanLiteralSyntax::BooleanLiteralSyntax(bool value)
    : SyntaxTemplate(value,
                     SyntaxCode::BooleanLiteral,
                     Format::Builder()
                         .set_is_literal(true)
                         .set_number_of_parameters(1)
                         .Build()) {}

BooleanLiteralSyntax::~BooleanLiteralSyntax() = default;

//
// NumericLiteralSyntax
//
NumericLiteralSyntax::NumericLiteralSyntax(float64_t value)
    : SyntaxTemplate(value,
                     SyntaxCode::NumericLiteral,
                     Format::Builder()
                         .set_is_literal(true)
                         .set_number_of_parameters(1)
                         .Build()) {}

NumericLiteralSyntax::~NumericLiteralSyntax() = default;

//
// NullLiteralSyntax
//
NullLiteralSyntax::NullLiteralSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::NullLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

NullLiteralSyntax::~NullLiteralSyntax() = default;

//
// StringLiteralSyntax
//
StringLiteralSyntax::StringLiteralSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::StringLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

StringLiteralSyntax::~StringLiteralSyntax() = default;

//
// UndefinedLiteralSyntax
//
UndefinedLiteralSyntax::UndefinedLiteralSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::UndefinedLiteral,
                     Format::Builder().set_is_literal(true).Build()) {}

UndefinedLiteralSyntax::~UndefinedLiteralSyntax() = default;

}  // namespace ast
}  // namespace joana

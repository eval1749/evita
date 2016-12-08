// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/literals.h"

#include "joana/public/ast/name.h"
#include "joana/public/escaped_string_piece.h"

namespace joana {
namespace ast {

//
// BooleanLiteral
//
BooleanLiteral::BooleanLiteral(const Name& name, bool value)
    : Literal(name.range()), value_(value) {}

BooleanLiteral::~BooleanLiteral() = default;

//
// NullLiteral
//
NullLiteral::NullLiteral(const Name& name) : Literal(name.range()) {}

NullLiteral::~NullLiteral() = default;

//
// Literal
//
Literal::Literal(const SourceCodeRange& range) : Node(range) {}

Literal::~Literal() = default;

//
// NumericLiteral
//
NumericLiteral::NumericLiteral(const SourceCodeRange& range, double value)
    : Literal(range), value_(value) {}

NumericLiteral::~NumericLiteral() = default;

// Implements |Node| members
void NumericLiteral::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", " << value_;
}

//
// StringLiteral
//
StringLiteral::StringLiteral(const SourceCodeRange& range,
                             base::StringPiece16 data)
    : Literal(range), data_(base::string16(data.data(), data.size())) {}

StringLiteral::~StringLiteral() = default;

// Implements |Node| members
void StringLiteral::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", '" << EscapedStringPiece16(data_, '\'') << '\'';
}

//
// UndefinedLiteral
//
UndefinedLiteral::UndefinedLiteral(const Name& name) : Literal(name.range()) {}

UndefinedLiteral::~UndefinedLiteral() = default;

}  // namespace ast
}  // namespace joana

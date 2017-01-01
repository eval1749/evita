// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/literals.h"

#include "joana/base/escaped_string_piece.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_1(Literal, BooleanLiteral, 0, bool, value)
IMPLEMENT_AST_SYNTAX_0(Literal, NullLiteral, 0)
IMPLEMENT_AST_SYNTAX_1(Literal, NumericLiteral, 0, float64_t, value)
IMPLEMENT_AST_SYNTAX_0(Literal, StringLiteral, 0)
IMPLEMENT_AST_SYNTAX_0(Literal, UndefinedLiteral, 0)

#if 0
//
// BooleanLiteral
//
BooleanLiteral::BooleanLiteral(const Node& name, bool value)
    : Literal(name.range()), value_(value) {}

BooleanLiteral::~BooleanLiteral() = default;

//
// Literal
//
Literal::Literal(const SourceCodeRange& range) : Token(range) {}

Literal::~Literal() = default;

//
// NullLiteral
//
NullLiteral::NullLiteral(const Node& name) : Literal(name.range()) {}

NullLiteral::~NullLiteral() = default;

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
UndefinedLiteral::UndefinedLiteral(const Node& name) : Literal(name.range()) {}

UndefinedLiteral::~UndefinedLiteral() = default;

#endif

}  // namespace ast
}  // namespace joana

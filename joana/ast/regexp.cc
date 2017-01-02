// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/regexp.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(RegExp, AnyCharRegExp, 0)
IMPLEMENT_AST_SYNTAX_1(RegExp, AssertionRegExp, 0, RegExpAssertionKind, kind)
IMPLEMENT_AST_SYNTAX_0(RegExp, CaptureRegExp, 1)
IMPLEMENT_AST_SYNTAX_0(RegExp, CharSetRegExp, 0)
IMPLEMENT_AST_SYNTAX_0(RegExp, ComplementCharSetRegExp, 0)
IMPLEMENT_AST_SYNTAX_0(RegExp, InvalidRegExp, 0)
IMPLEMENT_AST_SYNTAX_0(RegExp, LiteralRegExp, 0)
IMPLEMENT_AST_SYNTAX_0(RegExp, LookAheadRegExp, 1)
IMPLEMENT_AST_SYNTAX_0(RegExp, LookAheadNotRegExp, 1)
IMPLEMENT_AST_SYNTAX_0(RegExp, RepeatRegExp, 2)

//
// OrRegExpSyntax
//
OrRegExpSyntax::OrRegExpSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::OrRegExp,
                     Format::Builder().set_is_variadic(true).Build()) {}

OrRegExpSyntax::~OrRegExpSyntax() = default;

//
// RegExpRepeatSyntax
//
RegExpRepeatSyntax::RegExpRepeatSyntax(RegExpRepeatMethod method,
                                       int min,
                                       int max)
    : SyntaxTemplate(std::make_tuple(method, min, max),
                     SyntaxCode::RegExpRepeat,
                     Format::Builder().set_number_of_parameters(3).Build()) {}

RegExpRepeatSyntax::~RegExpRepeatSyntax() = default;

//
// SequenceRegExpSyntax
//
SequenceRegExpSyntax::SequenceRegExpSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::SequenceRegExp,
                     Format::Builder().set_is_variadic(true).Build()) {}

SequenceRegExpSyntax::~SequenceRegExpSyntax() = default;

}  // namespace ast
}  // namespace joana

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
IMPLEMENT_AST_SYNTAX_1(RegExp, GreedyRepeatRegExp, 1, RegExpRepeat, repeat)
IMPLEMENT_AST_SYNTAX_0(RegExp, InvalidRegExp, 0)
IMPLEMENT_AST_SYNTAX_1(RegExp, LazyRepeatRegExp, 1, RegExpRepeat, repeat)
IMPLEMENT_AST_SYNTAX_0(RegExp, LiteralRegExp, 0)
IMPLEMENT_AST_SYNTAX_0(RegExp, LookAheadRegExp, 1)
IMPLEMENT_AST_SYNTAX_0(RegExp, LookAheadNotRegExp, 1)

//
// RegExpRepeat
//
bool operator<(const RegExpRepeat& repeat1, const RegExpRepeat& repeat2) {
  if (repeat1.min != repeat2.min)
    return repeat1.min < repeat2.min;
  return repeat1.max < repeat2.max;
}

std::ostream& operator<<(std::ostream& ostream, const RegExpRepeat& repeat) {
  if (repeat.min == 0 && repeat.max == 1)
    return ostream << '?';
  if (repeat.min == 0 && repeat.max == ast::RegExpRepeat::kInfinity)
    return ostream << '*';
  if (repeat.min == 1 && repeat.max == ast::RegExpRepeat::kInfinity)
    return ostream << '+';
  if (repeat.min == repeat.max)
    return ostream << '{' << repeat.min << '}';
  if (repeat.max == ast::RegExpRepeat::kInfinity)
    return ostream << '{' << repeat.min << ",}";
  return ostream << '{' << repeat.min << ',' << repeat.max << '}';
}

//
// OrRegExpSyntax
//
OrRegExpSyntax::OrRegExpSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::OrRegExp,
                     Format::Builder().set_is_variadic(true).Build()) {}

OrRegExpSyntax::~OrRegExpSyntax() = default;

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

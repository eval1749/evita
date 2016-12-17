// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/regexp.h"

namespace joana {
namespace ast {

//
// AnyCharRegExp
//
AnyCharRegExp::AnyCharRegExp(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

AnyCharRegExp::~AnyCharRegExp() = default;

//
// AssertionRegExp
//
AssertionRegExp::AssertionRegExp(const SourceCodeRange& range,
                                 RegExpAssertionKind kind)
    : NodeTemplate(kind, range) {}

AssertionRegExp::~AssertionRegExp() = default;

//
// CaptureRegExp
//
CaptureRegExp::CaptureRegExp(const SourceCodeRange& range, RegExp* pattern)
    : NodeTemplate(pattern, range) {}

CaptureRegExp::~CaptureRegExp() = default;

//
// CharSetRegExp
//
CharSetRegExp::CharSetRegExp(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

CharSetRegExp::~CharSetRegExp() = default;

//
// ComplementCharSetRegExp
//
ComplementCharSetRegExp::ComplementCharSetRegExp(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

ComplementCharSetRegExp::~ComplementCharSetRegExp() = default;

//
// GreedyRepeatRegExp
//
GreedyRepeatRegExp::GreedyRepeatRegExp(const SourceCodeRange& range,
                                       RegExp* pattern,
                                       const RegExpRepeat& repeat)
    : NodeTemplate(std::make_tuple(pattern, repeat), range) {}

GreedyRepeatRegExp::~GreedyRepeatRegExp() = default;

//
// InvalidRegExp
//
InvalidRegExp::InvalidRegExp(const SourceCodeRange& range, int error_code)
    : RegExp(range), error_code_(error_code) {}

InvalidRegExp::~InvalidRegExp() = default;

//
// LazyRepeatRegExp
//
LazyRepeatRegExp::LazyRepeatRegExp(const SourceCodeRange& range,
                                   RegExp* pattern,
                                   const RegExpRepeat& repeat)
    : NodeTemplate(std::make_tuple(pattern, repeat), range) {}

LazyRepeatRegExp::~LazyRepeatRegExp() = default;

//
// LiteralRegExp
//
LiteralRegExp::LiteralRegExp(const SourceCodeRange& range)
    : NodeTemplate(std::make_tuple(), range) {}

LiteralRegExp::~LiteralRegExp() = default;

//
// LookAheadRegExp
//
LookAheadRegExp::LookAheadRegExp(const SourceCodeRange& range, RegExp* pattern)
    : NodeTemplate(pattern, range) {}

LookAheadRegExp::~LookAheadRegExp() = default;

//
// LookAheadNotRegExp
//
LookAheadNotRegExp::LookAheadNotRegExp(const SourceCodeRange& range,
                                       RegExp* pattern)
    : NodeTemplate(pattern, range) {}

LookAheadNotRegExp::~LookAheadNotRegExp() = default;

//
// RegExp
//
RegExp::RegExp(const SourceCodeRange& range) : Node(range) {}

RegExp::~RegExp() = default;

//
// RegExpList
//
RegExpList::RegExpList(Zone* zone, const std::vector<RegExp*>& patterns)
    : patterns_(zone, patterns) {}

RegExpList::~RegExpList() = default;

//
// OrRegExp
//
OrRegExp::OrRegExp(const SourceCodeRange& range, RegExpList* patterns)
    : NodeTemplate(patterns, range) {}

OrRegExp::~OrRegExp() = default;

//
// SequenceRegExp
//
SequenceRegExp::SequenceRegExp(const SourceCodeRange& range,
                               RegExpList* patterns)
    : NodeTemplate(patterns, range) {}

SequenceRegExp::~SequenceRegExp() = default;

//
// RegExpRepeat
//
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

}  // namespace ast
}  // namespace joana

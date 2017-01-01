// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_REGEXP_H_
#define JOANA_AST_REGEXP_H_

#include <limits>

#include "joana/ast/node.h"
#include "joana/ast/syntax.h"

namespace joana {
namespace ast {

#define FOR_EACH_AST_REGEXP_ASSERTION(V) \
  V(Boundary, "\\b")                     \
  V(BoundaryNot, "\\B")                  \
  V(End, "$")                            \
  V(Start, "^")

#define FOR_EACH_AST_REGEXP_FLAG(V) \
  V('g', GlobalMatch)               \
  V('i', IgnoreCase)                \
  V('m', Multiline)                 \
  V('u', Unicode)                   \
  V('y', Sticky)

#define FOR_EACH_AST_REGEXP_KNOWN_CHAR_SET(V) \
  V(Digit, "\\d")                             \
  V(DigitNot, "\\D")                          \
  V(Space, "\\s")                             \
  V(SpaceNot, "\\S")                          \
  V(Word, "\\w")                              \
  V(WordNot, "\\W")

//
// RegExpAssertionKind
//
enum class RegExpAssertionKind {
  Invalid,
#define V(name, string) name,
  FOR_EACH_AST_REGEXP_ASSERTION(V)
#undef V
};

//
// RegExpRepeat
//
struct JOANA_AST_EXPORT RegExpRepeat {
  static constexpr auto kInfinity = std::numeric_limits<int>::max();

  int min = 0;
  int max = 0;
};

JOANA_AST_EXPORT bool operator<(const RegExpRepeat& repeat1,
                                const RegExpRepeat& repeat2);

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const RegExpRepeat& repeat);

DECLARE_AST_SYNTAX_0(AnyCharRegExp)
DECLARE_AST_SYNTAX_1(AssertionRegExp, RegExpAssertionKind, kind)
DECLARE_AST_SYNTAX_0(CaptureRegExp)
DECLARE_AST_SYNTAX_0(CharSetRegExp)
DECLARE_AST_SYNTAX_0(ComplementCharSetRegExp)
DECLARE_AST_SYNTAX_1(GreedyRepeatRegExp, RegExpRepeat, repeat)
DECLARE_AST_SYNTAX_0(InvalidRegExp)
DECLARE_AST_SYNTAX_1(LazyRepeatRegExp, RegExpRepeat, repeat)
DECLARE_AST_SYNTAX_0(LiteralRegExp)
DECLARE_AST_SYNTAX_0(LookAheadRegExp)
DECLARE_AST_SYNTAX_0(LookAheadNotRegExp)
DECLARE_AST_SYNTAX_0(OrRegExp)
DECLARE_AST_SYNTAX_0(SequenceRegExp)

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_REGEXP_H_

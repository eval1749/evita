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

enum class RegExpRepeatMethod {
  Greedy,
  Lazy,
};

constexpr auto kRegExpInfinity = std::numeric_limits<int>::max();

DECLARE_AST_SYNTAX_0(AnyCharRegExp)
DECLARE_AST_SYNTAX_1(AssertionRegExp, RegExpAssertionKind, kind)
DECLARE_AST_SYNTAX_0(CaptureRegExp)
DECLARE_AST_SYNTAX_0(CharSetRegExp)
DECLARE_AST_SYNTAX_0(ComplementCharSetRegExp)
DECLARE_AST_SYNTAX_0(EmptyRegExp)
DECLARE_AST_SYNTAX_0(InvalidRegExp)
DECLARE_AST_SYNTAX_0(LiteralRegExp)
DECLARE_AST_SYNTAX_0(LookAheadRegExp)
DECLARE_AST_SYNTAX_0(LookAheadNotRegExp)
DECLARE_AST_SYNTAX_0(OrRegExp)
DECLARE_AST_SYNTAX_0(RepeatRegExp)
DECLARE_AST_SYNTAX_0(SequenceRegExp)

//
// RegExpRepeatSyntax
//
class JOANA_AST_EXPORT RegExpRepeatSyntax final
    : public SyntaxTemplate<Syntax, RegExpRepeatMethod, int, int> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpRepeat, Syntax);

 public:
  ~RegExpRepeatSyntax() final;

  bool is_lazy() const { return parameter_at<0>() == RegExpRepeatMethod::Lazy; }
  int max() const { return parameter_at<2>(); }
  int min() const { return parameter_at<1>(); }

 private:
  RegExpRepeatSyntax(RegExpRepeatMethod mode, int min, int max);

  DISALLOW_COPY_AND_ASSIGN(RegExpRepeatSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_REGEXP_H_

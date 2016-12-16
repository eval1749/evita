// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_REGEXP_H_
#define JOANA_PUBLIC_AST_REGEXP_H_

#include "joana/public/ast/node.h"

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
// RegExp
//
class JOANA_PUBLIC_EXPORT RegExp : public Node {
  DECLARE_ABSTRACT_AST_NODE(RegExp, Node);

 public:
  ~RegExp() override;

 protected:
  RegExp(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(RegExp);
};

//
// RegExpList
//
class JOANA_PUBLIC_EXPORT RegExpList final : public ZoneAllocated {
 public:
  ~RegExpList();

  auto begin() const { return expressions_.begin(); }
  bool empty() const { return expressions_.empty(); }
  auto end() const { return expressions_.end(); }
  size_t size() const { return expressions_.size(); }

 private:
  friend class NodeFactory;

  RegExpList(Zone* zone, const std::vector<RegExp*>& expressions);

  ZoneVector<RegExp*> expressions_;

  DISALLOW_COPY_AND_ASSIGN(RegExpList);
};

//
// AnyCharRegExp
//
class JOANA_PUBLIC_EXPORT AnyCharRegExp final : public NodeTemplate<RegExp> {
  DECLARE_CONCRETE_AST_NODE(AnyCharRegExp, RegExp);

 public:
  ~AnyCharRegExp() final;

 private:
  AnyCharRegExp(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(AnyCharRegExp);
};

//
// AssertionRegExp
//
class JOANA_PUBLIC_EXPORT AssertionRegExp final
    : public NodeTemplate<RegExp, RegExpAssertionKind> {
  DECLARE_CONCRETE_AST_NODE(AssertionRegExp, RegExp);

 public:
  ~AssertionRegExp() final;

  RegExpAssertionKind operation() const { return member_at<0>(); }

 private:
  AssertionRegExp(const SourceCodeRange& range, RegExpAssertionKind operation);

  DISALLOW_COPY_AND_ASSIGN(AssertionRegExp);
};

//
// CaptureRegExp
//
class JOANA_PUBLIC_EXPORT CaptureRegExp final
    : public NodeTemplate<RegExp, RegExp*> {
  DECLARE_CONCRETE_AST_NODE(CaptureRegExp, RegExp);

 public:
  ~CaptureRegExp() final;

  RegExp& pattern() const { return *member_at<0>(); }

 private:
  CaptureRegExp(const SourceCodeRange& range, RegExp* pattern);

  DISALLOW_COPY_AND_ASSIGN(CaptureRegExp);
};

//
// CharSetRegExp
//
class JOANA_PUBLIC_EXPORT CharSetRegExp final : public NodeTemplate<RegExp> {
  DECLARE_CONCRETE_AST_NODE(CharSetRegExp, RegExp);

 public:
  ~CharSetRegExp() final;

 private:
  CharSetRegExp(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(CharSetRegExp);
};

//
// ComplementCharSetRegExp
//
class JOANA_PUBLIC_EXPORT ComplementCharSetRegExp final
    : public NodeTemplate<RegExp> {
  DECLARE_CONCRETE_AST_NODE(ComplementCharSetRegExp, RegExp);

 public:
  ~ComplementCharSetRegExp() final;

 private:
  ComplementCharSetRegExp(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(ComplementCharSetRegExp);
};

//
// GreedyRepeatRegExp
//
class JOANA_PUBLIC_EXPORT GreedyRepeatRegExp final
    : public NodeTemplate<RegExp, RegExp*> {
  DECLARE_CONCRETE_AST_NODE(GreedyRepeatRegExp, RegExp);

 public:
  ~GreedyRepeatRegExp() final;

  RegExp& expression() const { return *member_at<0>(); }

 private:
  GreedyRepeatRegExp(const SourceCodeRange& range, RegExp* pattern);

  DISALLOW_COPY_AND_ASSIGN(GreedyRepeatRegExp);
};

//
// InvalidRegExp
//
class JOANA_PUBLIC_EXPORT InvalidRegExp : public RegExp {
  DECLARE_CONCRETE_AST_NODE(InvalidRegExp, RegExp);

 public:
  ~InvalidRegExp() final;

  int error_code() const { return error_code_; }

 private:
  InvalidRegExp(const Node& node, int error_code);

  const int error_code_;

  DISALLOW_COPY_AND_ASSIGN(InvalidRegExp);
};

//
// LazyRepeatRegExp
//
class JOANA_PUBLIC_EXPORT LazyRepeatRegExp final
    : public NodeTemplate<RegExp, RegExp*> {
  DECLARE_CONCRETE_AST_NODE(LazyRepeatRegExp, RegExp);

 public:
  ~LazyRepeatRegExp() final;

  RegExp& expression() const { return *member_at<0>(); }

 private:
  LazyRepeatRegExp(const SourceCodeRange& range, RegExp* pattern);

  DISALLOW_COPY_AND_ASSIGN(LazyRepeatRegExp);
};

//
// LookAheadRegExp
//
class JOANA_PUBLIC_EXPORT LookAheadRegExp final
    : public NodeTemplate<RegExp, RegExp*> {
  DECLARE_CONCRETE_AST_NODE(LookAheadRegExp, RegExp);

 public:
  ~LookAheadRegExp() final;

  RegExp& expression() const { return *member_at<0>(); }

 private:
  LookAheadRegExp(const SourceCodeRange& range, RegExp* pattern);

  DISALLOW_COPY_AND_ASSIGN(LookAheadRegExp);
};

//
// LookAheadNotRegExp
//
class JOANA_PUBLIC_EXPORT LookAheadNotRegExp final
    : public NodeTemplate<RegExp, RegExp*> {
  DECLARE_CONCRETE_AST_NODE(LookAheadNotRegExp, RegExp);

 public:
  ~LookAheadNotRegExp() final;

  RegExp& expression() const { return *member_at<0>(); }

 private:
  LookAheadNotRegExp(const SourceCodeRange& range, RegExp* pattern);

  DISALLOW_COPY_AND_ASSIGN(LookAheadNotRegExp);
};

//
// LiteralRegExp
//
class JOANA_PUBLIC_EXPORT LiteralRegExp final : public NodeTemplate<RegExp> {
  DECLARE_CONCRETE_AST_NODE(LiteralRegExp, RegExp);

 public:
  ~LiteralRegExp() final;

 private:
  LiteralRegExp(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(LiteralRegExp);
};

//
// OrRegExp
//
class JOANA_PUBLIC_EXPORT OrRegExp final
    : public NodeTemplate<RegExp, RegExpList*> {
  DECLARE_CONCRETE_AST_NODE(OrRegExp, RegExp);

 public:
  ~OrRegExp() final;

  const RegExpList& elements() const { return *member_at<0>(); }

 private:
  OrRegExp(const SourceCodeRange& range, RegExpList* elements);

  DISALLOW_COPY_AND_ASSIGN(OrRegExp);
};

//
// SequenceRegExp
//
class JOANA_PUBLIC_EXPORT SequenceRegExp final
    : public NodeTemplate<RegExp, RegExpList*> {
  DECLARE_CONCRETE_AST_NODE(SequenceRegExp, RegExp);

 public:
  ~SequenceRegExp() final;

  const RegExpList& elements() const { return *member_at<0>(); }

 private:
  SequenceRegExp(const SourceCodeRange& range, RegExpList* elements);

  DISALLOW_COPY_AND_ASSIGN(SequenceRegExp);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_REGEXP_H_

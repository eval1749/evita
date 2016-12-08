// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_H_
#define JOANA_PUBLIC_AST_LITERALS_H_

#include "joana/public/ast/node.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Literal : public Node {
  DECLARE_ABSTRACT_AST_NODE(Literal, Node);

 public:
  ~Literal() override;

 protected:
  explicit Literal(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

class JOANA_PUBLIC_EXPORT BooleanLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(BooleanLiteral, Literal);

 public:
  ~BooleanLiteral() final;

  bool value() const { return value_; }

 private:
  BooleanLiteral(const SourceCodeRange& range, bool value);

  const bool value_;

  DISALLOW_COPY_AND_ASSIGN(BooleanLiteral);
};

class JOANA_PUBLIC_EXPORT NullLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(NullLiteral, Literal);

 public:
  ~NullLiteral() final;

 private:
  explicit NullLiteral(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

class JOANA_PUBLIC_EXPORT NumericLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(NumericLiteral, Literal);

 public:
  ~NumericLiteral() final;

  double value() const { return value_; }

 private:
  NumericLiteral(const SourceCodeRange& range, double value);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const double value_;

  DISALLOW_COPY_AND_ASSIGN(NumericLiteral);
};

class JOANA_PUBLIC_EXPORT StringLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(StringLiteral, Literal);

 public:
  ~StringLiteral() final;

  const base::string16& data() const { return data_; }

 private:
  StringLiteral(const SourceCodeRange& range, base::StringPiece16 data);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(StringLiteral);
};

class JOANA_PUBLIC_EXPORT UndefinedLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(UndefinedLiteral, Literal);

 public:
  ~UndefinedLiteral() final;

 private:
  explicit UndefinedLiteral(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(UndefinedLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_H_

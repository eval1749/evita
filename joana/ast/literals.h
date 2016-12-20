// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_LITERALS_H_
#define JOANA_AST_LITERALS_H_

#include "joana/ast/tokens.h"

namespace joana {
namespace ast {

class Name;

class JOANA_AST_EXPORT Literal : public Token {
  DECLARE_ABSTRACT_AST_NODE(Literal, Token);

 public:
  ~Literal() override;

 protected:
  explicit Literal(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

class JOANA_AST_EXPORT BooleanLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(BooleanLiteral, Literal);

 public:
  ~BooleanLiteral() final;

  bool value() const { return value_; }

 private:
  BooleanLiteral(const Name& name, bool value);

  const bool value_;

  DISALLOW_COPY_AND_ASSIGN(BooleanLiteral);
};

class JOANA_AST_EXPORT NullLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(NullLiteral, Literal);

 public:
  ~NullLiteral() final;

 private:
  explicit NullLiteral(const Name& name);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

class JOANA_AST_EXPORT NumericLiteral final : public Literal {
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

class JOANA_AST_EXPORT StringLiteral final : public Literal {
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

class JOANA_AST_EXPORT UndefinedLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(UndefinedLiteral, Literal);

 public:
  ~UndefinedLiteral() final;

 private:
  explicit UndefinedLiteral(const Name& name);

  DISALLOW_COPY_AND_ASSIGN(UndefinedLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_LITERALS_H_

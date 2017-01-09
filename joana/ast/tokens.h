// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_TOKENS_H_
#define JOANA_AST_TOKENS_H_

#include <iosfwd>

#include "joana/ast/node.h"

#include "joana/ast/jsdoc_tags.h"
#include "joana/ast/lexical_grammar.h"
#include "joana/ast/syntax.h"

namespace joana {
namespace ast {

//
// TokenKind
//
enum class TokenKind {
#define V(text, capital, upper, category) capital,
  FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V

      StartOfKeyword,
#define V(name, camel, upper) camel,
  FOR_EACH_JAVASCRIPT_KEYWORD(V)
#undef V
      EndOfKeyword,

  StartOfKnownWord,
#define V(name, camel, upper) camel,
  FOR_EACH_JAVASCRIPT_KNOWN_WORD(V)
#undef V
      EndOfKnownWord,

  StartOfJsDocTagName,
#define V(name, camel, syntax) At##camel,
  FOR_EACH_JSDOC_TAG_NAME(V)
#undef V
      EndOfJsDocTagName,
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          TokenKind kind);

//
// Token
// Note: For ease of debugging, |Token| hold |TokenKind| even if it will be
// out of range.
class JOANA_AST_EXPORT Token : public SyntaxTemplate<Syntax, TokenKind> {
  DECLARE_ABSTRACT_AST_SYNTAX(Token, Syntax);

 public:
  ~Token() override;

  TokenKind kind() const { return parameter_at<0>(); }
  int number() const { return static_cast<int>(kind()); }

  static int IdOf(const Node& node);
  static TokenKind KindOf(const Node& node);

 protected:
  Token(SyntaxCode syntax_code, TokenKind kind);

 private:
  DISALLOW_COPY_AND_ASSIGN(Token);
};

DECLARE_AST_SYNTAX_0(Comment)
DECLARE_AST_SYNTAX_0(Empty)
DECLARE_AST_SYNTAX_1(Invalid, int, error_code)

//
// Name
//
class JOANA_AST_EXPORT Name final : public Token {
  DECLARE_CONCRETE_AST_SYNTAX(Name, Token);

 public:
  ~Name() final;

  static bool IsKeyword(const Node& node);

 private:
  explicit Name(int number);

  DISALLOW_COPY_AND_ASSIGN(Name);
};

//
// Punctuator
//
class JOANA_AST_EXPORT Punctuator final : public Token {
  DECLARE_CONCRETE_AST_SYNTAX(Punctuator, Token);

 public:
  ~Punctuator() final;

  TokenKind kind() const { return static_cast<TokenKind>(number()); }

 private:
  explicit Punctuator(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(Punctuator);
};

//
// RegExpSource
//
class JOANA_AST_EXPORT RegExpSource final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpSource, Syntax);

 public:
  ~RegExpSource() final;

 private:
  RegExpSource();

  DISALLOW_COPY_AND_ASSIGN(RegExpSource);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TOKENS_H_

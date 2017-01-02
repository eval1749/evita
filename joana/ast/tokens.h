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
#define V(name, camel, syntax) JsDoc##camel,
  FOR_EACH_JSDOC_TAG_NAME(V)
#undef V
      EndOfJsDocTagName,
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          TokenKind kind);

//
// Token
//
class JOANA_AST_EXPORT Token : public SyntaxTemplate<Syntax, int> {
  DECLARE_ABSTRACT_AST_SYNTAX(Token, Syntax);

 public:
  ~Token() override;

  int number() const { return parameter_at<0>(); }

  static int IdOf(const Node& node);
  static TokenKind KindOf(const Node& node);

 protected:
  Token(SyntaxCode syntax_code, int number);

 private:
  DISALLOW_COPY_AND_ASSIGN(Token);
};

DECLARE_AST_SYNTAX_0(Comment)
DECLARE_AST_SYNTAX_0(Empty)
DECLARE_AST_SYNTAX_1(Invalid, int, error_code)

//
// NameSyntax
//
class JOANA_AST_EXPORT NameSyntax final : public Token {
  DECLARE_CONCRETE_AST_SYNTAX(Name, Token);

 public:
  ~NameSyntax() final;

  static bool IsKeyword(const Node& node);

 private:
  explicit NameSyntax(int number);

  DISALLOW_COPY_AND_ASSIGN(NameSyntax);
};

//
// PunctuatorSyntax
//
class JOANA_AST_EXPORT PunctuatorSyntax final : public Token {
  DECLARE_CONCRETE_AST_SYNTAX(Punctuator, Token);

 public:
  ~PunctuatorSyntax() final;

  TokenKind kind() const { return static_cast<TokenKind>(number()); }

 private:
  explicit PunctuatorSyntax(TokenKind kind);

  DISALLOW_COPY_AND_ASSIGN(PunctuatorSyntax);
};

//
// RegExpSourceSyntax
//
class JOANA_AST_EXPORT RegExpSourceSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpSource, Syntax);

 public:
  ~RegExpSourceSyntax() final;

 private:
  RegExpSourceSyntax();

  DISALLOW_COPY_AND_ASSIGN(RegExpSourceSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TOKENS_H_

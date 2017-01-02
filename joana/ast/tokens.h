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
// PunctuatorKind
//
enum class PunctuatorKind {
#define V(text, capital, upper, category) capital,
  FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
};

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          PunctuatorKind kind);

//
// NameId
//
enum class NameId {
  Invalid,

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

DECLARE_AST_SYNTAX_0(Comment)
DECLARE_AST_SYNTAX_0(Empty)
DECLARE_AST_SYNTAX_1(Invalid, int, error_code)

//
// NameSyntax
//
class JOANA_AST_EXPORT NameSyntax final : public SyntaxTemplate<Syntax, int> {
  DECLARE_CONCRETE_AST_SYNTAX(NameSyntax, Syntax);

 public:
  ~NameSyntax() final;

  int number() const { return parameter_at<0>(); }

  static int IdOf(const Node& node);
  static bool IsKeyword(const Node& node);

 private:
  explicit NameSyntax(int number);

  DISALLOW_COPY_AND_ASSIGN(NameSyntax);
};

//
// PunctuatorSyntax
//
class JOANA_AST_EXPORT PunctuatorSyntax final
    : public SyntaxTemplate<Syntax, PunctuatorKind> {
  DECLARE_CONCRETE_AST_SYNTAX(PunctuatorSyntax, Syntax);

 public:
  ~PunctuatorSyntax() final;

  PunctuatorKind kind() const { return parameter_at<0>(); }

  static PunctuatorKind KindOf(const Node& node);

 private:
  explicit PunctuatorSyntax(PunctuatorKind kind);

  DISALLOW_COPY_AND_ASSIGN(PunctuatorSyntax);
};

//
// RegExpSourceSyntax
//
class JOANA_AST_EXPORT RegExpSourceSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(RegExpSourceSyntax, Syntax);

 public:
  ~RegExpSourceSyntax() final;

 private:
  RegExpSourceSyntax();

  DISALLOW_COPY_AND_ASSIGN(RegExpSourceSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TOKENS_H_

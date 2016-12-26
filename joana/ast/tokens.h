// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_TOKENS_H_
#define JOANA_AST_TOKENS_H_

#include "joana/ast/node.h"

#include "joana/ast/jsdoc_tags.h"
#include "joana/ast/lexical_grammar.h"

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

//
// Token
//
class JOANA_AST_EXPORT Token : public Node {
  DECLARE_ABSTRACT_AST_NODE(Token, Node);

 public:
  ~Token() override;

 protected:
  explicit Token(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Token);
};

//
// Comment
//
class JOANA_AST_EXPORT Comment final : public Token {
  DECLARE_CONCRETE_AST_NODE(Comment, Token);

 public:
  ~Comment() final;

 private:
  explicit Comment(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(Comment);
};

//
// Empty is used for representing anonymous class and function.
//
class JOANA_AST_EXPORT Empty final : public Token {
  DECLARE_CONCRETE_AST_NODE(Empty, Token);

 public:
  ~Empty() final;

 private:
  explicit Empty(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(Empty);
};

//
// JsDoc
//
class JOANA_AST_EXPORT JsDoc final : public Token {
  DECLARE_CONCRETE_AST_NODE(JsDoc, Token);

 public:
  ~JsDoc() final;

 private:
  explicit JsDoc(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(JsDoc);
};

//
// Name
//
class JOANA_AST_EXPORT Name final : public Token {
  DECLARE_CONCRETE_AST_NODE(Name, Token);

 public:
  ~Name() final;

  int number() const { return number_; }

  bool IsKeyword() const;

 private:
  Name(const SourceCodeRange& range, int number);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const int number_;

  DISALLOW_COPY_AND_ASSIGN(Name);
};

//
// Punctuator
//
class JOANA_AST_EXPORT Punctuator final : public Token {
  DECLARE_CONCRETE_AST_NODE(Punctuator, Node);

 public:
  explicit Punctuator(const SourceCodeRange& range, PunctuatorKind kind);
  ~Punctuator() final;

  PunctuatorKind kind() const { return kind_; }

 private:
  const PunctuatorKind kind_;

  DISALLOW_COPY_AND_ASSIGN(Punctuator);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TOKENS_H_

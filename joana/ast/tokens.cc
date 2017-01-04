// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/tokens.h"

#include "joana/ast/error_codes.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Token, Comment, 0)
IMPLEMENT_AST_SYNTAX_0(Token, Empty, 0)
IMPLEMENT_AST_SYNTAX_1(Token, Invalid, 0, int, error_code)

//
// Name
//
Name::Name(int number)
    : Token(SyntaxCode::Name, static_cast<TokenKind>(number)) {}
Name::~Name() = default;

bool Name::IsKeyword(const Node& node) {
  const auto number = IdOf(node);
  return number > static_cast<int>(TokenKind::StartOfKeyword) &&
         number < static_cast<int>(TokenKind::EndOfKeyword);
}

//
// Punctuator
//
Punctuator::Punctuator(TokenKind kind) : Token(SyntaxCode::Punctuator, kind) {}

Punctuator::~Punctuator() = default;

//
// RegExpSource
//
RegExpSource::RegExpSource()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RegExpSource,
                     Syntax::Format()) {}

RegExpSource::~RegExpSource() = default;

//
// Token
//
Token::Token(SyntaxCode syntax_code, TokenKind kind)
    : SyntaxTemplate(
          std::make_tuple(kind),
          syntax_code,
          Syntax::Format::Builder().set_number_of_parameters(1).Build()) {}

Token::~Token() = default;

int Token::IdOf(const Node& node) {
  return node.syntax().As<Token>().number();
}

TokenKind Token::KindOf(const Node& node) {
  return node.syntax().As<Token>().kind();
}

}  // namespace ast
}  // namespace joana

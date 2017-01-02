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
// NameSyntax
//
NameSyntax::NameSyntax(int number) : Token(SyntaxCode::Name, number) {}
NameSyntax::~NameSyntax() = default;

bool NameSyntax::IsKeyword(const Node& node) {
  const auto number = IdOf(node);
  return number > static_cast<int>(TokenKind::StartOfKeyword) &&
         number < static_cast<int>(TokenKind::EndOfKeyword);
}

//
// PunctuatorSyntax
//
PunctuatorSyntax::PunctuatorSyntax(TokenKind kind)
    : Token(SyntaxCode::Punctuator, static_cast<int>(kind)) {}

PunctuatorSyntax::~PunctuatorSyntax() = default;

//
// RegExpSourceSyntax
//
RegExpSourceSyntax::RegExpSourceSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::RegExpSource,
                     Syntax::Format()) {}

RegExpSourceSyntax::~RegExpSourceSyntax() = default;

//
// Token
//
Token::Token(SyntaxCode syntax_code, int number)
    : SyntaxTemplate(
          std::make_tuple(number),
          syntax_code,
          Syntax::Format::Builder().set_number_of_parameters(1).Build()) {}

Token::~Token() = default;

int Token::IdOf(const Node& node) {
  return node.syntax().As<Token>().number();
}

TokenKind Token::KindOf(const Node& node) {
  return static_cast<TokenKind>(IdOf(node));
}

}  // namespace ast
}  // namespace joana

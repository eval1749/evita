// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "joana/ast/tokens.h"

#include "joana/ast/error_codes.h"

namespace joana {
namespace ast {

IMPLEMENT_AST_SYNTAX_0(Token, Comment, 0)
IMPLEMENT_AST_SYNTAX_0(Token, Empty, 0)
IMPLEMENT_AST_SYNTAX_1(Token, Invalid, 0, int, error_code)
IMPLEMENT_AST_SYNTAX_1(Token, Punctuator, 0, PunctuatorKind, kind)
IMPLEMENT_AST_SYNTAX_1(Token, Name, 0, int, number)

PunctuatorKind PunctuatorSyntax::KindOf(const Node& node) {
  return node.syntax().As<PunctuatorSyntax>().kind();
}

//
// NameSyntax
//
int NameSyntax::IdOf(const Node& node) {
  return node.syntax().As<NameSyntax>().number();
}

bool NameSyntax::IsKeyword(const Node& node) {
  const auto number = IdOf(node);
  return number > static_cast<int>(NameId::StartOfKeyword) &&
         number < static_cast<int>(NameId::EndOfKeyword);
}

}  // namespace ast
}  // namespace joana

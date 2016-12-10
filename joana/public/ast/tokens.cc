// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/tokens.h"

#include "joana/public/ast/error_codes.h"

namespace joana {
namespace ast {

//
// Comment
//
Comment::Comment(const SourceCodeRange& range) : Token(range) {}

Comment::~Comment() = default;

//
// Invalid
//
Invalid::Invalid(const SourceCodeRange& range, int error_code)
    : Token(range), error_code_(error_code) {}

Invalid::~Invalid() = default;

// Implements |Node| members
void Invalid::PrintMoreTo(std::ostream* ostream) const {
  static const char* kLiteralErrorTexts[] = {
#define V(token, reason) "LEXER_" #token "_" #reason,
      FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
  };

  static const char* kParserErrorText[] = {
#define V(token, reason) "PASER_" #token "_" #reason,
      FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
  };

  *ostream << ", ";
  const auto lexer_it =
      std::begin(kLiteralErrorTexts) + error_code_ - kLexerErrorCodeBase - 1;
  if (lexer_it >= std::begin(kLiteralErrorTexts) &&
      lexer_it < std::end(kLiteralErrorTexts)) {
    *ostream << *lexer_it;
    return;
  }
  const auto parser_it =
      std::begin(kParserErrorText) + error_code_ - kParserErrorCodeBase - 1;
  if (parser_it >= std::begin(kParserErrorText) &&
      parser_it < std::end(kParserErrorText)) {
    *ostream << *parser_it;
    return;
  }
  *ostream << error_code_;
}

//
// Name
//
Name::Name(const SourceCodeRange& range, int number)
    : Token(range), number_(number) {}

Name::~Name() = default;

bool Name::IsKeyword() const {
  return number_ > static_cast<int>(NameId::StartOfKeyword) &&
         number_ < static_cast<int>(NameId::EndOfKeyword);
}

// Implements |Node| members
void Name::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", " << number_;
}

//
// Punctuator
//
Punctuator::Punctuator(const SourceCodeRange& range, PunctuatorKind kind)
    : Token(range), kind_(kind) {}

Punctuator::~Punctuator() = default;

//
// Token
//
Token::Token(const SourceCodeRange& range) : Node(range) {}
Token::~Token() = default;

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "joana/ast/tokens.h"

#include "joana/ast/error_codes.h"

namespace joana {
namespace ast {

//
// Comment
//
Comment::Comment(const SourceCodeRange& range) : Token(range) {}

Comment::~Comment() = default;

//
// Empty
//
Empty::Empty(const SourceCodeRange& range) : Token(range) {}

Empty::~Empty() = default;

//
// Invalid
//
Invalid::Invalid(const SourceCodeRange& range, int error_code)
    : Token(range), error_code_(error_code) {}

Invalid::~Invalid() = default;

// Implements |Node| members
void Invalid::PrintMoreTo(std::ostream* ostream) const {
  *ostream << ", ";
  const auto string = ErrorStringOf(error_code_);
  if (string.empty()) {
    *ostream << error_code_;
    return;
  }
  *ostream << string;
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

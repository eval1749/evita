// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/public/ast/invalid.h"

#include "joana/public/ast/error_codes.h"

namespace joana {
namespace ast {

Invalid::Invalid(const SourceCodeRange& range, int error_code)
    : Node(range), error_code_(error_code) {}

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

}  // namespace ast
}  // namespace joana

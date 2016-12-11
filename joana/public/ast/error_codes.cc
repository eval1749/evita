// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/public/ast/error_codes.h"

namespace joana {
namespace ast {

base::StringPiece ErrorStringOf(int error_code) {
  static const char* kLiteralErrorTexts[] = {
#define V(token, reason) "LEXER_ERROR_" #token "_" #reason,
      FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
  };

  static const char* kParserErrorText[] = {
#define V(token, reason) "PASER_ERROR_" #token "_" #reason,
      FOR_EACH_PARSER_ERROR_CODE(V)
#undef V
  };

  const auto lexer_it =
      std::begin(kLiteralErrorTexts) + error_code - kLexerErrorCodeBase - 1;
  if (lexer_it >= std::begin(kLiteralErrorTexts) &&
      lexer_it < std::end(kLiteralErrorTexts)) {
    return base::StringPiece(*lexer_it);
  }
  const auto parser_it =
      std::begin(kParserErrorText) + error_code - kParserErrorCodeBase - 1;
  if (parser_it >= std::begin(kParserErrorText) &&
      parser_it < std::end(kParserErrorText)) {
    return base::StringPiece(*parser_it);
  }
  return base::StringPiece();
}

}  // namespace ast
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/ast/error_codes.h"

namespace joana {
namespace ast {

base::StringPiece ErrorStringOf(int error_code) {
  static const char* kJsDocErrorText[] = {
#define V(token, reason) "JSDOC_ERROR_" #token "_" #reason,
      FOR_EACH_JSDOC_ERROR_CODE(V)
#undef V
  };

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

  static const char* kRegExpErrorText[] = {
#define V(token, reason) "REGEXP_ERROR_" #token "_" #reason,
      FOR_EACH_REGEXP_ERROR_CODE(V)
#undef V
  };

  static const char* kTypeErrorText[] = {
#define V(token, reason) "TYPE_ERROR_" #token "_" #reason,
      FOR_EACH_TYPE_ERROR_CODE(V)
#undef V
  };

  const auto jsdoc_it =
      std::begin(kJsDocErrorText) + error_code - kJsDocErrorCodeBase - 1;
  if (jsdoc_it >= std::begin(kJsDocErrorText) &&
      jsdoc_it < std::end(kJsDocErrorText)) {
    return base::StringPiece(*jsdoc_it);
  }
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
  const auto regexp_it =
      std::begin(kRegExpErrorText) + error_code - kRegExpErrorCodeBase - 1;
  if (regexp_it >= std::begin(kRegExpErrorText) &&
      regexp_it < std::end(kRegExpErrorText)) {
    return base::StringPiece(*regexp_it);
  }
  const auto type_it =
      std::begin(kTypeErrorText) + error_code - kTypeErrorCodeBase - 1;
  if (type_it >= std::begin(kTypeErrorText) &&
      type_it < std::end(kTypeErrorText)) {
    return base::StringPiece(*type_it);
  }
  return base::StringPiece("(no message)");
}

}  // namespace ast
}  // namespace joana

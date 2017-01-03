// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/error_codes.h"

#include "joana/ast/error_codes.h"

namespace joana {
namespace analyzer {

base::StringPiece ErrorStringOf(int error_code) {
  static const char* kAnalyzerErrorTexts[] = {
#define V(token, reason) "ANALYZER_ERROR_" #token "_" #reason,
      FOR_EACH_ANALYZER_ERROR_CODE(V)
#undef V
  };
  const auto analyzer_it =
      std::begin(kAnalyzerErrorTexts) + error_code - kAnalyzerErrorCodeBase - 1;
  if (analyzer_it >= std::begin(kAnalyzerErrorTexts) &&
      analyzer_it < std::end(kAnalyzerErrorTexts)) {
    return base::StringPiece(*analyzer_it);
  }
  return ast::ErrorStringOf(error_code);
}

}  // namespace analyzer
}  // namespace joana

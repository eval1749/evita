// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_
#define JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_

#include "base/strings/string_piece.h"
#include "joana/analyzer/public/analyzer_export.h"

namespace joana {

// V(pass, reason)
#define FOR_EACH_ANALYZER_ERROR_CODE(V) \
  V(ENVIRONMENT, EXPECT_METHOD)         \
  V(ENVIRONMENT, MULTIPLE_BINDINGS)     \
  V(TYPE_CHECKER, UNDEFIEND_VARIABLE)

const auto kAnalyzerErrorCodeBase = 70000;

namespace analyzer {

JOANA_ANALYZER_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_

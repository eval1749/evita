// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ERROR_CODES_H_
#define JOANA_ANALYZER_ERROR_CODES_H_

#include "joana/analyzer/public/error_codes.h"

namespace joana {
namespace analyzer {

enum class ErrorCode {
  Start = kAnalyzerErrorCodeBase,
#define V(pass, reason) pass##_##reason,
  FOR_EACH_ANALYZER_ERROR_CODE(V)
#undef V
      End,
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ERROR_CODES_H_

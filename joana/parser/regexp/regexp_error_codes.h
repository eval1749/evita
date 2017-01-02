// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_REGEXP_REGEXP_ERROR_CODES_H_
#define JOANA_PARSER_REGEXP_REGEXP_ERROR_CODES_H_

#include "joana/ast/error_codes.h"

namespace joana {
namespace parser {

enum class RegExpErrorCode {
  None = ast::kRegExpErrorCodeBase,
#define V(category, reason) category##_##reason,
  FOR_EACH_REGEXP_ERROR_CODE(V)
#undef V
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_REGEXP_REGEXP_ERROR_CODES_H_

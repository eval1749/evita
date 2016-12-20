// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_ERROR_CODES_H_
#define JOANA_PARSER_PARSER_ERROR_CODES_H_

#include "joana/ast/error_codes.h"

#include "joana/parser/parser.h"

namespace joana {
namespace internal {

enum class Parser::ErrorCode {
  None = ast::kParserErrorCodeBase,
#define V(category, reason) ERROR_##category##_##reason,
  FOR_EACH_PARSER_ERROR_CODE(V)
#undef V
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_ERROR_CODES_H_

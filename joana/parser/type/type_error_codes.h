// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_TYPE_TYPE_ERROR_CODES_H_
#define JOANA_PARSER_TYPE_TYPE_ERROR_CODES_H_

#include "joana/ast/error_codes.h"

namespace joana {
namespace parser {

enum class TypeErrorCode {
  None = ast::kTypeErrorCodeBase,
#define V(category, reason) ERROR_##category##_##reason,
  FOR_EACH_TYPE_ERROR_CODE(V)
#undef V
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_TYPE_TYPE_ERROR_CODES_H_

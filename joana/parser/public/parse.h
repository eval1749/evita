// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSE_H_
#define JOANA_PARSER_PUBLIC_PARSE_H_

#include "base/macros.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_export.h"
#include "joana/parser/public/parser_options.h"

namespace joana {

namespace ast {
class Node;
}

class SourceCodeRange;

//
// The parser entry point.
//
JOANA_PARSER_EXPORT const ast::Node& Parse(ParserContext* context,
                                           const SourceCodeRange& range,
                                           const ParserOptions& options);

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSE_H_

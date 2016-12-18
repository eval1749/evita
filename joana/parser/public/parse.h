// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSE_H_
#define JOANA_PARSER_PUBLIC_PARSE_H_

#include "joana/parser/public/parser_export.h"

namespace joana {

namespace ast {
class EditContext;
class Node;
}

class SourceCodeRange;

//
// ParserOptions
//
JOANA_PARSER_EXPORT struct ParserOptions {
  // Disable automatic semicolon insertion.
  bool disable_automatic_semicolon = false;
};

JOANA_PARSER_EXPORT const ast::Node& Parse(ast::EditContext* context,
                                           const SourceCodeRange& range,
                                           const ParserOptions& options);

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSE_H_

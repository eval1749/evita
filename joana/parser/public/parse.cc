// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parse.h"

#include "joana/parser/parser.h"

namespace joana {

using Parser = parser::Parser;

//
// Parse; the entry point
//
const ast::Node& Parse(ParserContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options) {
  Parser parser(context, range, options);
  return parser.Run();
}

}  // namespace joana

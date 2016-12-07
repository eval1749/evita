// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parse.h"

#include "joana/parser/parser.h"

namespace joana {

using Parser = internal::Parser;

const ast::Node& Parse(ast::EditContext* context,
                       const SourceCodeRange& range) {
  Parser parser(context, range);
  return parser.Run();
}

}  // namespace joana

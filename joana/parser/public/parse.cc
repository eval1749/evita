// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parse.h"

#include "joana/parser/parser.h"

namespace joana {

using Parser = internal::Parser;

const ast::Node& Parse(ast::NodeFactory* node_factory,
                       ErrorSink* error_sink,
                       const SourceCodeRange& location) {
  Parser parser(node_factory, error_sink, location);
  return parser.Run();
}

}  // namespace joana

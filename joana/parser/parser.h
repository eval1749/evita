// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>

#include "base/macros.h"

namespace joana {
namespace ast {
class Node;
class NodeFactory;
}
class ErrorSink;
class SourceCodeRange;

namespace internal {

class Lexer;

class Parser final {
 public:
  Parser(ast::NodeFactory* node_factory,
         ErrorSink* error_sink,
         const SourceCodeRange& range);

  ~Parser();

  const ast::Node& Run();

 private:
  ErrorSink* const error_sink_;
  std::unique_ptr<Lexer> lexer_;
  ast::NodeFactory* const node_factory_;
  ast::Node& root_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_

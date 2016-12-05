// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_LEXER_H_
#define JOANA_PARSER_LEXER_LEXER_H_

#include <memory>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
class NodeFactory;
}

class ErrorSink;
class SourceCode;
class SourceCodeRange;

namespace internal {

class CharacterReader;

class Lexer final {
 public:
  Lexer(ast::NodeFactory* node_factory,
        ErrorSink* error_sink,
        const SourceCodeRange& range);

  ~Lexer();

  bool HasMore() const;
  const ast::Node& NextToken();

 private:
  const SourceCode& source_code() const;

  ErrorSink* const error_sink_;
  ast::NodeFactory* const node_factory_;
  const SourceCodeRange& range_;
  const std::unique_ptr<CharacterReader> reader_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_LEXER_H_

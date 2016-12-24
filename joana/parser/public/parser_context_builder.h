// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSER_CONTEXT_BUILDER_H_
#define JOANA_PARSER_PUBLIC_PARSER_CONTEXT_BUILDER_H_

#include <memory>

#include "joana/parser/public/parser_context.h"

namespace joana {

namespace ast {
class NodeFactory;
}

//
// ParserContext::Builder
//
class JOANA_PARSER_EXPORT ParserContext::Builder {
 public:
  Builder();
  ~Builder();

  std::unique_ptr<ParserContext> Build() const;

  Builder& set_error_sink(ErrorSink* error_sink);
  Builder& set_node_factory(ast::NodeFactory* node_factory);

 private:
  friend class ParserContext;

  ErrorSink* error_sink_ = nullptr;
  ast::NodeFactory* node_factory_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSER_CONTEXT_BUILDER_H_

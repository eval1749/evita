// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parser_context.h"

#include "joana/parser/public/parser_context_builder.h"

namespace joana {

//
// ParserContext
//
ParserContext::ParserContext(const Builder& builder)
    : error_sink_(*builder.error_sink_),
      node_factory_(*builder.node_factory_) {}

ParserContext::~ParserContext() = default;

ErrorSink& ParserContext::error_sink() const {
  return error_sink_;
}
ast::NodeFactory& ParserContext::node_factory() const {
  return node_factory_;
}

}  // namespace joana

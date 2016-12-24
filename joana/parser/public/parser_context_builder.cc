// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "joana/parser/public/parser_context_builder.h"

#include "base/logging.h"

namespace joana {

//
// ParserContext::Builder
//
ParserContext::Builder::Builder() = default;
ParserContext::Builder::~Builder() = default;

std::unique_ptr<ParserContext> ParserContext::Builder::Build() const {
  DCHECK(error_sink_);
  DCHECK(node_factory_);
  return std::move(std::unique_ptr<ParserContext>(new ParserContext(*this)));
}

ParserContext::Builder& ParserContext::Builder::set_error_sink(
    ErrorSink* error_sink) {
  DCHECK(error_sink);
  error_sink_ = error_sink;
  return *this;
}

ParserContext::Builder& ParserContext::Builder::set_node_factory(
    ast::NodeFactory* node_factory) {
  DCHECK(node_factory);
  node_factory_ = node_factory;
  return *this;
}

}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/node_factory.h"

namespace joana {
namespace internal {

Parser::Parser(ast::NodeFactory* node_factory,
               ErrorSink* error_sink,
               const SourceCodeRange& range)
    : error_sink_(error_sink),
      lexer_(new Lexer(node_factory, error_sink, range)),
      node_factory_(node_factory),
      root_(node_factory_->NewModule(range)) {}

Parser::~Parser() = default;

const ast::Node& Parser::Run() {
  return root_;
}

}  // namespace internal
}  // namespace joana

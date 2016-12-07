// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/invalid.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/node_factory.h"

namespace joana {
namespace internal {

Parser::Parser(ast::EditContext* context, const SourceCodeRange& range)
    : context_(context),
      lexer_(new Lexer(context, range)),
      root_(context->node_factory()->NewModule(range)) {}

Parser::~Parser() = default;

const ast::Node& Parser::Run() {
  while (lexer_->HasToken()) {
    auto& token = lexer_->GetToken();
    if (token.is<ast::Invalid>()) {
      // TODO(eval1749): We should skip tokens until good point to restart
      // toplevel parsing.
      lexer_->Advance();
      continue;
    }
    lexer_->Advance();
  }
  return root_;
}

}  // namespace internal
}  // namespace joana

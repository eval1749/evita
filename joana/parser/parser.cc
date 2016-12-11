// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/error_sink.h"
#include "joana/public/source_code.h"

namespace joana {
namespace internal {

Parser::Parser(ast::EditContext* context, const SourceCodeRange& range)
    : context_(context),
      lexer_(new Lexer(context, range)),
      root_(context->node_factory().NewModule(range)) {}

Parser::~Parser() = default;

ast::NodeFactory& Parser::node_factory() const {
  return context_->node_factory();
}

const SourceCode& Parser::source_code() const {
  return lexer_->source_code();
}

void Parser::AddError(const ast::Node& token, ErrorCode error_code) {
  AddError(token.range(), error_code);
}

void Parser::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  context_->error_sink().AddError(range, static_cast<int>(error_code));
}

void Parser::Advance() {
  lexer_->Advance();
}

ast::Node& Parser::ConsumeToken() {
  auto& token = PeekToken();
  Advance();
  return token;
}

bool Parser::ConsumeTokenIf(ast::NameId name_id) {
  if (!HasToken())
    return false;
  if (PeekToken() != name_id)
    return false;
  Advance();
  return true;
}

bool Parser::ConsumeTokenIf(ast::PunctuatorKind kind) {
  if (!HasToken())
    return false;
  if (PeekToken() != kind)
    return false;
  Advance();
  return true;
}

ast::Node& Parser::PeekToken() {
  return lexer_->PeekToken();
}

bool Parser::HasToken() const {
  return lexer_->HasToken();
}

const ast::Node& Parser::Run() {
  while (HasToken()) {
    auto& token = PeekToken();
    if (token.Is<ast::Comment>()) {
      Advance();
      continue;
    }
    if (token.Is<ast::Invalid>()) {
      // TODO(eval1749): We should skip tokens until good point to restart
      // toplevel parsing.
      Advance();
      continue;
    }
    ast::NodeEditor().AppendChild(&root_, &ParseStatement());
  }
  return root_;
}

}  // namespace internal
}  // namespace joana

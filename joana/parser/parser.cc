// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
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

//
// BracketStack
//
class Parser::BracketStack final {
 public:
  BracketStack() = default;
  ~BracketStack() = default;

  const std::vector<const ast::Node*>& tokens() const { return stack_; }

  ErrorCode Feed(const ast::Node& token);

 private:
  ErrorCode Check(const ast::Node& token, ast::PunctuatorKind expected);

  std::vector<const ast::Node*> stack_;
  DISALLOW_COPY_AND_ASSIGN(BracketStack);
};

Parser::ErrorCode Parser::BracketStack::Check(const ast::Node& token,
                                              ast::PunctuatorKind expected) {
  if (stack_.empty())
    return ErrorCode::ERROR_BRACKET_UNEXPECTED;
  if (*stack_.back() != expected)
    return ErrorCode::ERROR_BRACKET_MISMATCHED;
  stack_.pop_back();
  return ErrorCode::None;
}

Parser::ErrorCode Parser::BracketStack::Feed(const ast::Node& token) {
  auto* const bracket = token.TryAs<ast::Punctuator>();
  if (!bracket)
    return ErrorCode::None;
  switch (static_cast<ast::PunctuatorKind>(bracket->kind())) {
    case ast::PunctuatorKind::LeftParenthesis:
    case ast::PunctuatorKind::LeftBracket:
    case ast::PunctuatorKind::LeftBrace:
      stack_.push_back(bracket);
      return ErrorCode::None;
    case ast::PunctuatorKind::RightParenthesis:
      return Check(token, ast::PunctuatorKind::LeftParenthesis);
    case ast::PunctuatorKind::RightBracket:
      return Check(token, ast::PunctuatorKind::LeftBracket);
    case ast::PunctuatorKind::RightBrace:
      return Check(token, ast::PunctuatorKind::LeftBrace);
  }
  return ErrorCode::None;
}

//
// Parser
//
Parser::Parser(ast::EditContext* context, const SourceCodeRange& range)
    : bracket_stack_(new BracketStack()),
      context_(context),
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

void Parser::AddError(ErrorCode error_code) {
  AddError(ComputeInvalidToken(error_code), error_code);
}

void Parser::Advance() {
  lexer_->Advance();
  if (!HasToken())
    return;
  const auto error_code = bracket_stack_->Feed(PeekToken());
  if (error_code == ErrorCode::None)
    return;
  AddError(PeekToken(), error_code);
}

ast::Node& Parser::ConsumeToken() {
  auto& token = PeekToken();
  Advance();
  return token;
}

ast::Node& Parser::ComputeInvalidToken(ErrorCode error_code) {
  if (HasToken())
    return PeekToken();
  return node_factory().NewInvalid(source_code().end(),
                                   static_cast<int>(error_code));
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

void Parser::ExpectToken(ast::NameId name_id, ErrorCode error_code) {
  if (ConsumeTokenIf(name_id))
    return;
  if (HasToken())
    return AddError(PeekToken(), error_code);
  return AddError(lexer_->location(), error_code);
}

void Parser::ExpectToken(ast::PunctuatorKind kind, ErrorCode error_code) {
  if (ConsumeTokenIf(kind))
    return;
  if (HasToken())
    return AddError(PeekToken(), error_code);
  return AddError(lexer_->location(), error_code);
}

ast::Node& Parser::PeekToken() {
  return lexer_->PeekToken();
}

bool Parser::HasToken() const {
  return lexer_->HasToken();
}

const ast::Node& Parser::Run() {
  if (HasToken())
    bracket_stack_->Feed(PeekToken());
  while (HasToken()) {
    auto& token = PeekToken();
    if (token.Is<ast::Comment>()) {
      ast::NodeEditor().AppendChild(&root_, &ConsumeToken());
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
  for (const auto& bracket : bracket_stack_->tokens())
    AddError(*bracket, ErrorCode::ERROR_BRACKET_NOT_CLOSED);
  return root_;
}

}  // namespace internal
}  // namespace joana

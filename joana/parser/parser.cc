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
// SourceCodeRangeScope
//
Parser::SourceCodeRangeScope::SourceCodeRangeScope(Parser* parser)
    : parser_(parser) {
  parser_->range_stack_.push(parser_->PeekToken().range().start());
}

Parser::SourceCodeRangeScope::~SourceCodeRangeScope() {
  parser_->range_stack_.pop();
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
  if (!token_stack_.empty()) {
    token_stack_.pop();
    return;
  }
  lexer_->Advance();
  if (!CanPeekToken())
    return;
  auto& token = PeekToken();
  tokens_.push_back(&token);
  const auto error_code = bracket_stack_->Feed(token);
  if (error_code == ErrorCode::None)
    return;
  AddError(token, error_code);
}

bool Parser::CanPeekToken() const {
  if (!token_stack_.empty())
    return true;
  while (lexer_->CanPeekToken() && PeekToken().Is<ast::Comment>())
    lexer_->Advance();
  return lexer_->CanPeekToken();
}

ast::Token& Parser::ConsumeToken() {
  auto& token = PeekToken();
  Advance();
  return token;
}

ast::Token& Parser::ComputeInvalidToken(ErrorCode error_code) {
  if (CanPeekToken())
    return PeekToken();
  return node_factory().NewInvalid(source_code().end(),
                                   static_cast<int>(error_code));
}

bool Parser::ConsumeTokenIf(ast::NameId name_id) {
  if (!CanPeekToken())
    return false;
  if (PeekToken() != name_id)
    return false;
  Advance();
  return true;
}

bool Parser::ConsumeTokenIf(ast::PunctuatorKind kind) {
  if (!CanPeekToken())
    return false;
  if (PeekToken() != kind)
    return false;
  Advance();
  return true;
}

void Parser::ExpectToken(ast::NameId name_id, ErrorCode error_code) {
  if (ConsumeTokenIf(name_id))
    return;
  if (CanPeekToken())
    return AddError(PeekToken(), error_code);
  return AddError(lexer_->location(), error_code);
}

void Parser::ExpectToken(ast::PunctuatorKind kind, ErrorCode error_code) {
  if (ConsumeTokenIf(kind))
    return;
  if (CanPeekToken())
    return AddError(PeekToken(), error_code);
  return AddError(lexer_->location(), error_code);
}

SourceCodeRange Parser::GetSourceCodeRange() const {
  return source_code().Slice(range_stack_.top(), lexer_->location().end());
}

ast::Token& Parser::NewEmptyName() {
  return node_factory().NewEmpty(lexer_->location());
}

ast::Token& Parser::PeekToken() const {
  if (token_stack_.empty())
    return lexer_->PeekToken();
  return *token_stack_.top();
}

void Parser::PushBackToken(const ast::Token& token) {
  token_stack_.push(const_cast<ast::Token*>(&token));
}

const ast::Node& Parser::Run() {
  if (CanPeekToken())
    bracket_stack_->Feed(PeekToken());
  while (CanPeekToken()) {
    auto& token = PeekToken();
    if (token.Is<ast::Invalid>()) {
      // TODO(eval1749): We should skip tokens until good point to restart
      // toplevel parsing.
      Advance();
      continue;
    }
    SourceCodeRangeScope(this);
    ast::NodeEditor().AppendChild(&root_, &ParseStatement());
  }
  for (const auto& bracket : bracket_stack_->tokens())
    AddError(*bracket, ErrorCode::ERROR_BRACKET_NOT_CLOSED);
  return root_;
}

}  // namespace internal
}  // namespace joana

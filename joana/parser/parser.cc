// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <vector>

#include "joana/parser/parser.h"

#include "joana/ast/edit_context.h"
#include "joana/ast/module.h"
#include "joana/ast/node_editor.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"

namespace joana {
namespace internal {

//
// BracketStack
//
class Parser::BracketStack final {
 public:
  explicit BracketStack(ErrorSink* error_sink);
  ~BracketStack() = default;

  const std::vector<const ast::Node*>& tokens() const { return stack_; }

  void Feed(const ast::Node& token);

 private:
  void Check(const ast::Node& token, ast::PunctuatorKind expected);

  ErrorSink& error_sink_;
  std::vector<const ast::Node*> stack_;

  DISALLOW_COPY_AND_ASSIGN(BracketStack);
};

Parser::BracketStack::BracketStack(ErrorSink* error_sink)
    : error_sink_(*error_sink) {}

void Parser::BracketStack::Check(const ast::Node& token,
                                 ast::PunctuatorKind expected) {
  if (stack_.empty()) {
    error_sink_.AddError(token.range(),
                         static_cast<int>(ErrorCode::ERROR_BRACKET_UNEXPECTED));
    return;
  }
  const auto& start_token = *stack_.back();
  if (start_token == expected) {
    stack_.pop_back();
    return;
  }
  error_sink_.AddError(
      SourceCodeRange::Merge(SourceCodeRange::CollapseToStart(token.range()),
                             start_token.range()),
      static_cast<int>(ErrorCode::ERROR_BRACKET_MISMATCHED));
}

void Parser::BracketStack::Feed(const ast::Node& token) {
  auto* const bracket = token.TryAs<ast::Punctuator>();
  if (!bracket)
    return;
  switch (static_cast<ast::PunctuatorKind>(bracket->kind())) {
    case ast::PunctuatorKind::LeftParenthesis:
    case ast::PunctuatorKind::LeftBracket:
    case ast::PunctuatorKind::LeftBrace:
      stack_.push_back(bracket);
      return;
    case ast::PunctuatorKind::RightParenthesis:
      return Check(token, ast::PunctuatorKind::LeftParenthesis);
    case ast::PunctuatorKind::RightBracket:
      return Check(token, ast::PunctuatorKind::LeftBracket);
    case ast::PunctuatorKind::RightBrace:
      return Check(token, ast::PunctuatorKind::LeftBrace);
  }
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
Parser::Parser(ast::EditContext* context,
               const SourceCodeRange& range,
               const ParserOptions& options)
    : bracket_stack_(new BracketStack(&context->error_sink())),
      context_(*context),
      lexer_(new Lexer(context, range, options)),
      options_(options),
      root_(context->node_factory().NewModule(range)) {}

Parser::~Parser() = default;

ast::NodeFactory& Parser::node_factory() const {
  return context_.node_factory();
}

const SourceCode& Parser::source_code() const {
  return lexer_->source_code();
}

void Parser::AddError(const ast::Node& token, ErrorCode error_code) {
  AddError(token.range(), error_code);
}

void Parser::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  if (range.IsCollapsed())
    DCHECK_EQ(range, source_code().end());
  context_.error_sink().AddError(range, static_cast<int>(error_code));
}

void Parser::AddError(ErrorCode error_code) {
  AddError(ComputeInvalidToken(error_code), error_code);
}

void Parser::Advance() {
  if (!token_stack_.empty()) {
    token_stack_.pop();
    return;
  }
  is_separated_by_newline_ = false;
  if (!lexer_->CanPeekToken())
    return;
  lexer_->Advance();
  SkipCommentTokens();
}

bool Parser::CanPeekToken() const {
  if (!token_stack_.empty())
    return true;
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

void Parser::ExpectPunctuator(ast::PunctuatorKind kind, ErrorCode error_code) {
  if (ConsumeTokenIf(kind))
    return;
  if (!CanPeekToken()) {
    // Unfinished statement or expression, e.g. just "foo" in source code.
    return AddError(
        source_code().Slice(range_stack_.top(), tokens_.back()->range().end()),
        error_code);
  }
  // We use previous token instead of current token, since current token may
  // be next line, e.g.
  // {
  //    foo()
  //    ~~~~~
  // }
  return AddError(
      source_code().Slice(range_stack_.top(),
                          (*std::next(tokens_.rbegin()))->range().end()),
      error_code);
}

void Parser::ExpectSemicolon() {
  if (!options_.disable_automatic_semicolon) {
    if (!CanPeekToken())
      return;
    if (ConsumeTokenIf(ast::PunctuatorKind::Semicolon))
      return;
    if (PeekToken() == ast::PunctuatorKind::RightBrace)
      return;
    if (is_separated_by_newline_)
      return;
  }
  ExpectPunctuator(ast::PunctuatorKind::Semicolon,
                   ErrorCode::ERROR_STATEMENT_EXPECT_SEMICOLON);
}

SourceCodeRange Parser::GetSourceCodeRange() const {
  return source_code().Slice(
      range_stack_.top(),
      CanPeekToken() ? PeekToken().range().end() : lexer_->location().end());
}

const ast::Token& Parser::NewEmptyName() {
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
  SkipCommentTokens();
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

void Parser::SkipCommentTokens() {
  while (lexer_->CanPeekToken()) {
    if (lexer_->is_separated_by_newline())
      is_separated_by_newline_ = true;
    bracket_stack_->Feed(PeekToken());
    tokens_.push_back(&PeekToken());
    if (!PeekToken().Is<ast::Comment>())
      return;
    lexer_->Advance();
  }
}
}  // namespace internal
}  // namespace joana

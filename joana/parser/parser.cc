// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <utility>
#include <vector>

#include "joana/parser/parser.h"

#include "joana/ast/jsdoc_nodes.h"
#include "joana/ast/module.h"
#include "joana/ast/node_editor.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/utils/bracket_tracker.h"

namespace joana {
namespace parser {

namespace {

bool IsCloseBracket(const ast::Token& token) {
  return token == ast::PunctuatorKind::RightBrace ||
         token == ast::PunctuatorKind::RightBracket ||
         token == ast::PunctuatorKind::RightParenthesis;
}

// Returns true if |document| contains |@fileoviewview| tag.
bool IsFileOverview(const ast::JsDocDocument& document) {
  for (const auto& element : document.elements()) {
    const auto* tag = element.TryAs<ast::JsDocTag>();
    if (tag && tag->name() == ast::NameId::JsDocFileOverview)
      return true;
  }
  return false;
}

std::unique_ptr<BracketTracker> NewBracketTracker(
    ErrorSink* error_sink,
    const SourceCodeRange& source_code_range) {
  const auto descriptions = std::vector<BracketTracker::Description>{
      {ast::PunctuatorKind::LeftParenthesis,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_EXPECT_RPAREN),
       ast::PunctuatorKind::RightParenthesis,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_UNEXPECT_RPAREN)},
      {ast::PunctuatorKind::LeftBrace,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_EXPECT_RBRACE),
       ast::PunctuatorKind::RightBrace,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_UNEXPECT_RBRACE)},
      {ast::PunctuatorKind::LeftBracket,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_EXPECT_RBRACKET),
       ast::PunctuatorKind::RightBracket,
       static_cast<int>(Parser::ErrorCode::ERROR_BRACKET_UNEXPECT_RBRACKET)},
  };

  return std::make_unique<BracketTracker>(error_sink, source_code_range,
                                          descriptions);
}

}  // namespace

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
Parser::Parser(ParserContext* context,
               const SourceCodeRange& range,
               const ParserOptions& options)
    : bracket_tracker_(NewBracketTracker(&context->error_sink(), range)),
      context_(*context),
      lexer_(new Lexer(context, range, options)),
      options_(options) {}

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
  if (CanPeekToken())
    return AddError(PeekToken(), error_code);
  return AddError(source_code().end(), error_code);
}

void Parser::Advance() {
  if (!token_stack_.empty()) {
    token_stack_.pop();
    return;
  }
  is_separated_by_newline_ = false;
  if (!lexer_->CanPeekToken())
    return;
  lexer_->ConsumeToken();
  SkipCommentTokens();
}

void Parser::AssociateJsDoc(const ast::JsDoc& js_doc, const ast::Node& node) {
  const auto& result = js_doc_map_.emplace(&node, &js_doc);
  if (result.second)
    return;
  if (IsFileOverview(js_doc.document()))
    return;
  // We don't allow statement/expression has more than one annotation.
  AddError(js_doc, ErrorCode::ERROR_STATEMENT_UNEXPECT_ANNOTATION);
}

bool Parser::CanPeekToken() const {
  if (!token_stack_.empty())
    return true;
  return lexer_->CanPeekToken();
}

const ast::Token& Parser::ConsumeToken() {
  auto& token = PeekToken();
  Advance();
  return token;
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
  if (!options_.disable_automatic_semicolon()) {
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

void Parser::Finish() {
  bracket_tracker_->Finish();
}

SourceCodeRange Parser::GetSourceCodeRange() const {
  return source_code().Slice(
      range_stack_.top(),
      CanPeekToken() ? PeekToken().range().end() : lexer_->location().end());
}

const ast::Token& Parser::NewEmptyName() {
  return node_factory().NewEmpty(lexer_->location());
}

const ast::Token& Parser::PeekToken() const {
  if (token_stack_.empty())
    return lexer_->PeekToken();
  return *token_stack_.top();
}

void Parser::PushBackToken(const ast::Token& token) {
  token_stack_.push(const_cast<ast::Token*>(&token));
}

const ast::Node& Parser::Run() {
  std::vector<const ast::Statement*> statements;
  SkipCommentTokens();
  while (CanPeekToken()) {
    auto& token = PeekToken();
    if (!token.Is<ast::JsDoc>()) {
      statements.push_back(&ParseStatement());
      continue;
    }
    auto& js_doc = ConsumeToken().As<ast::JsDoc>();
    auto& statement = ParseStatement();
    AssociateJsDoc(js_doc, statement);
    statements.push_back(&statement);
  }
  Finish();
  return node_factory().NewModule(source_code().range(), statements,
                                  js_doc_map_);
}

void Parser::SkipCommentTokens() {
  while (lexer_->CanPeekToken()) {
    if (lexer_->is_separated_by_newline())
      is_separated_by_newline_ = true;
    bracket_tracker_->Feed(PeekToken());
    tokens_.push_back(&PeekToken());
    if (!PeekToken().Is<ast::Comment>())
      return;
    lexer_->ConsumeToken();
  }
}

bool Parser::SkipToListElement() {
  const auto current_depth = bracket_tracker_->depth();
  DCHECK_GT(current_depth, 0) << "We should call SkipListElement() in list.";
  const auto close_bracket = bracket_tracker_->close_bracket();
  while (CanPeekToken()) {
    if (current_depth != bracket_tracker_->depth()) {
      ConsumeToken();
      continue;
    }
    if (IsCloseBracket(PeekToken()))
      return false;
    if (ConsumeToken() == ast::PunctuatorKind::Comma)
      return true;
  }
  return false;
}

}  // namespace parser
}  // namespace joana

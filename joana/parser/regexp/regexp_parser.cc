// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/parser/regexp/regexp_parser.h"

#include "joana/ast/node_factory.h"
#include "joana/ast/regexp.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/public/parse.h"
#include "joana/parser/regexp/regexp_error_codes.h"
#include "joana/parser/regexp/regexp_lexer.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/parser/utils/lexer_utils.h"

namespace joana {
namespace parser {

//
// RegExpParser::ScopedNodeFactory
//
class RegExpParser::ScopedNodeFactory final {
 public:
  explicit ScopedNodeFactory(RegExpParser* parser);
  ~ScopedNodeFactory();

  void AddError(const SourceCodeRange& range, RegExpErrorCode error_code);
  void AddError(RegExpErrorCode error_code);
  const ast::Node& NewCapture(const ast::Node& pattern);
  const ast::Node& NewError(RegExpErrorCode error_code);
  const ast::Node& NewOr(const std::vector<const ast::Node*> members);
  const ast::Node& NewRepeat(const ast::Node& pattern, const ast::Node& repeat);
  const ast::Node& NewSequence(const std::vector<const ast::Node*> members);

 private:
  ast::NodeFactory& factory() { return parser_.node_factory(); }

  SourceCodeRange ComputeRange() const;
  const ast::Node& NewInvalid(RegExpErrorCode error_code);

  RegExpParser& parser_;
  const int start_;

  DISALLOW_COPY_AND_ASSIGN(ScopedNodeFactory);
};

RegExpParser::ScopedNodeFactory::ScopedNodeFactory(RegExpParser* parser)
    : parser_(*parser), start_(parser->PeekToken().range().start()) {}

RegExpParser::ScopedNodeFactory::~ScopedNodeFactory() = default;

void RegExpParser::ScopedNodeFactory::AddError(const SourceCodeRange& range,
                                               RegExpErrorCode error_code) {
  parser_.context().error_sink().AddError(range, static_cast<int>(error_code));
}

void RegExpParser::ScopedNodeFactory::AddError(RegExpErrorCode error_code) {
  AddError(ComputeRange(), error_code);
}

SourceCodeRange RegExpParser::ScopedNodeFactory::ComputeRange() const {
  return parser_.source_code().Slice(start_,
                                     parser_.last_token_->range().end());
}

const ast::Node& RegExpParser::ScopedNodeFactory::NewCapture(
    const ast::Node& pattern) {
  return factory().NewCaptureRegExp(ComputeRange(), pattern);
}

const ast::Node& RegExpParser::ScopedNodeFactory::NewError(
    RegExpErrorCode error_code) {
  AddError(error_code);
  return NewInvalid(error_code);
}

const ast::Node& RegExpParser::ScopedNodeFactory::NewInvalid(
    RegExpErrorCode error_code) {
  return factory().NewInvalidRegExp(ComputeRange(),
                                    static_cast<int>(error_code));
}

const ast::Node& RegExpParser::ScopedNodeFactory::NewOr(
    const std::vector<const ast::Node*> members) {
  DCHECK(!members.empty());
  if (members.size() == 1)
    return *members.front();
  return factory().NewOrRegExp(ComputeRange(), members);
}

const ast::Node& RegExpParser::ScopedNodeFactory::NewRepeat(
    const ast::Node& pattern,
    const ast::Node& repeat) {
  DCHECK_EQ(repeat, ast::SyntaxCode::RegExpRepeat);
  return factory().NewRepeatRegExp(ComputeRange(), pattern, repeat);
}

// Note: /(?:)/ calls |NewSequence()| with |members.size() == 0|.
const ast::Node& RegExpParser::ScopedNodeFactory::NewSequence(
    const std::vector<const ast::Node*> members) {
  if (members.size() == 1)
    return *members.front();
  return factory().NewSequenceRegExp(ComputeRange(), members);
}

//
// RegExpParser
//
RegExpParser::RegExpParser(ParserContext* context,
                           const SourceCodeRange& range,
                           const ParserOptions& options)
    : context_(*context),
      lexer_(new RegExpLexer(context, range, options)),
      options_(options) {}

RegExpParser::~RegExpParser() = default;

// The entry point
const ast::Node& RegExpParser::Parse() {
  return ParseOr();
}

ast::NodeFactory& RegExpParser::node_factory() const {
  return context_.node_factory();
}

const SourceCode& RegExpParser::source_code() const {
  return lexer_->source_code();
}

const ast::Node& RegExpParser::ParseOr() {
  ScopedNodeFactory factory(this);
  std::vector<const ast::Node*> members;
  if (CanPeekToken() && PeekToken() == ast::TokenKind::BitOr)
    members.push_back(&NewEmpty(PeekToken().range()));
  else
    members.push_back(&ParseSequence());
  while (CanPeekToken() && ConsumeTokenIf(ast::TokenKind::BitOr)) {
    if (!CanPeekToken()) {
      members.push_back(&NewEmpty(
          source_code().Slice(lexer_->location(), lexer_->location())));
      break;
    }
    if (PeekToken() == ast::TokenKind::RightParenthesis) {
      members.push_back(&NewEmpty(PeekToken().range()));
      break;
    }
    if (PeekToken() == ast::TokenKind::BitOr) {
      members.push_back(&NewEmpty(ConsumeToken().range()));
      continue;
    }
    members.push_back(&ParseSequence());
  }
  return factory.NewOr(members);
}

const ast::Node& RegExpParser::ParsePrimary() {
  ScopedNodeFactory factory(this);
  if (PeekToken() == ast::SyntaxCode::AnyCharRegExp)
    return ConsumeToken();

  if (PeekToken() == ast::SyntaxCode::AssertionRegExp)
    return ConsumeToken();

  if (PeekToken() == ast::SyntaxCode::CharSetRegExp)
    return ConsumeToken();

  if (PeekToken() == ast::SyntaxCode::ComplementCharSetRegExp)
    return ConsumeToken();

  if (PeekToken() == ast::SyntaxCode::LiteralRegExp)
    return ConsumeToken();

  if (ConsumeTokenIf(ast::TokenKind::LeftParenthesis))
    return factory.NewCapture(ParseParenthesis());

  if (ConsumeTokenIf(ast::TokenKind::Colon))
    return ParseParenthesis();

  const auto& token = ConsumeToken();
  if (options_.enable_strict_regexp())
    return factory.NewError(RegExpErrorCode::REGEXP_EXPECT_PRIMARY);
  return node_factory().NewLiteralRegExp(token.range());
}

const ast::Node& RegExpParser::ParseParenthesis() {
  ScopedNodeFactory factory(this);
  if (ConsumeTokenIf(ast::TokenKind::RightParenthesis)) {
    // In case of /(?:)/
    return factory.NewSequence({});
  }
  groups_.push(last_token_->range().start());
  const auto& pattern = ParseOr();
  DCHECK(!groups_.empty());
  if (!ConsumeTokenIf(ast::TokenKind::RightParenthesis)) {
    factory.AddError(
        source_code().Slice(groups_.top(), last_token_->range().end()),
        RegExpErrorCode::REGEXP_EXPECT_RPAREN);
  }
  groups_.pop();
  return pattern;
}

const ast::Node& RegExpParser::ParseRepeat() {
  ScopedNodeFactory factory(this);
  auto& pattern = ParsePrimary();
  if (!CanPeekToken())
    return pattern;
  const auto& repeat = PeekToken();
  if (!ConsumeTokenIf(ast::SyntaxCode::RegExpRepeat))
    return pattern;
  return factory.NewRepeat(pattern, repeat);
}

bool CanMergeNodes(const ast::Node& pattern1, const ast::Node& pattern2) {
  return pattern1 == ast::SyntaxCode::LiteralRegExp &&
         pattern2 == ast::SyntaxCode::LiteralRegExp;
}

const ast::Node& RegExpParser::ParseSequence() {
  ScopedNodeFactory factory(this);
  std::vector<const ast::Node*> patterns;
  patterns.push_back(&ParseRepeat());
  while (CanPeekToken() && PeekToken() != ast::TokenKind::BitOr) {
    if (!CanPeekToken())
      break;
    if (PeekToken() == ast::TokenKind::RightParenthesis && !groups_.empty())
      break;
    auto& pattern = ParseRepeat();
    if (!patterns.empty() && CanMergeNodes(*patterns.back(), pattern)) {
      auto& merged = node_factory().NewLiteralRegExp(
          SourceCodeRange::Merge(patterns.back()->range(), pattern.range()));
      patterns.pop_back();
      patterns.push_back(&merged);
      continue;
    }
    patterns.push_back(&pattern);
  }
  return factory.NewSequence(patterns);
}

// Factory functions
const ast::Node& RegExpParser::NewEmpty(const SourceCodeRange& range) {
  return node_factory().NewEmptyRegExp(range);
}

// Lexer helper functions
bool RegExpParser::CanPeekToken() const {
  return lexer_->CanPeekToken();
}

const ast::Node& RegExpParser::ConsumeToken() {
  last_token_ = &PeekToken();
  return lexer_->ConsumeToken();
}

template <typename T>
bool RegExpParser::ConsumeTokenIf(T expected) {
  static_assert(std::is_enum<T>::value, "Should be SyntaxCode or TokenKind");
  if (!CanPeekToken() || PeekToken() != expected)
    return false;
  ConsumeToken();
  return true;
}

const ast::Node& RegExpParser::PeekToken() const {
  return lexer_->PeekToken();
}

}  // namespace parser
}  // namespace joana

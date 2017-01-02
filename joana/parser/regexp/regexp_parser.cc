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

#if 0
namespace {

//
// ScopedNodeFactory
//
class ScopedNodeFactory final {
 public:
  explicit ScopedNodeFactory(RegExpParser* parser);
  ~ScopedNodeFactory();

  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);
  const ast::Node& NewAnyChar();
  const ast::Node& NewAssertion(ast::RegExpAssertionKind kind);
  const ast::Node& NewCapture(const ast::Node& pattern);
  const ast::Node& NewCharSet();
  const ast::Node& NewComplementCharSet();
  const ast::Node& NewError(ErrorCode error_code);
  const ast::Node& NewInvalid(ErrorCode error_code);
  const ast::Node& NewGreedyRepeat(const ast::Node& pattern,
                                   const ast::RegExpRepeat& repeat);
  const ast::Node& NewLazyRepeat(const ast::Node& pattern,
                                 const ast::RegExpRepeat& repeat);
  const ast::Node& NewLookAhead(const ast::Node& pattern);
  const ast::Node& NewLookAheadNot(const ast::Node& pattern);
  const ast::Node& NewLiteral();
  const ast::Node& NewOr(const std::vector<const ast::Node*> members);
  const ast::Node& NewSequence(const std::vector<const ast::Node*> members);

  void SetToken(const Token& token);

 private:
  ast::NodeFactory& factory() { return parser_.node_factory(); }

  SourceCodeRange ComputeRange() const;

  RegExpParser& parser_;
  const int start_;
  int end_;

  DISALLOW_COPY_AND_ASSIGN(ScopedNodeFactory);
};

ScopedNodeFactory::ScopedNodeFactory(RegExpParser* parser)
    : parser_(*parser),
      start_(parser->PeekToken().start),
      end_(parser->PeekToken().end) {}

ScopedNodeFactory::~ScopedNodeFactory() = default;

void ScopedNodeFactory::AddError(const SourceCodeRange& range,
                                 ErrorCode error_code) {
  parser_.context().error_sink().AddError(range, static_cast<int>(error_code));
}

void ScopedNodeFactory::AddError(ErrorCode error_code) {
  AddError(ComputeRange(), error_code);
}

SourceCodeRange ScopedNodeFactory::ComputeRange() const {
  DCHECK_NE(start_, end_);
  return parser_.source_code().Slice(start_, end_);
}

const ast::Node& ScopedNodeFactory::NewAnyChar() {
  return factory().NewAnyCharRegExp(ComputeRange());
}

const ast::Node& ScopedNodeFactory::NewAssertion(
    ast::RegExpAssertionKind kind) {
  return factory().NewAssertionRegExp(ComputeRange(), kind);
}

const ast::Node& ScopedNodeFactory::NewCapture(const ast::Node& pattern) {
  return factory().NewCaptureRegExp(ComputeRange(), pattern);
}

const ast::Node& ScopedNodeFactory::NewCharSet() {
  return factory().NewCharSetRegExp(ComputeRange());
}

const ast::Node& ScopedNodeFactory::NewComplementCharSet() {
  return factory().NewComplementCharSetRegExp(ComputeRange());
}

const ast::Node& ScopedNodeFactory::NewError(ErrorCode error_code) {
  AddError(error_code);
  return NewInvalid(error_code);
}

const ast::Node& ScopedNodeFactory::NewGreedyRepeat(
    const ast::Node& pattern,
    const ast::RegExpRepeat& repeat) {
  return factory().NewGreedyRepeatRegExp(ComputeRange(), pattern, repeat);
}

const ast::Node& ScopedNodeFactory::NewInvalid(ErrorCode error_code) {
  return factory().NewInvalidRegExp(ComputeRange(),
                                    static_cast<int>(error_code));
}

const ast::Node& ScopedNodeFactory::NewLazyRepeat(
    const ast::Node& pattern,
    const ast::RegExpRepeat& repeat) {
  return factory().NewLazyRepeatRegExp(ComputeRange(), pattern, repeat);
}

const ast::Node& ScopedNodeFactory::NewLookAhead(const ast::Node& pattern) {
  return factory().NewLookAheadRegExp(ComputeRange(), pattern);
}

const ast::Node& ScopedNodeFactory::NewLookAheadNot(const ast::Node& pattern) {
  return factory().NewLookAheadNotRegExp(ComputeRange(), pattern);
}

const ast::Node& ScopedNodeFactory::NewLiteral() {
  return factory().NewLiteralRegExp(ComputeRange());
}

const ast::Node& ScopedNodeFactory::NewOr(
    const std::vector<const ast::Node*> members) {
  if (members.size() == 1)
    return *members.front();
  if (members.size() >= 2)
    return factory().NewOrRegExp(ComputeRange(), members);
  return NewError(ErrorCode::REGEXP_INVALID_OR);
}

// Note: /(?:)/ calls |NewSequence()| with |members.size() == 0|.
const ast::Node& ScopedNodeFactory::NewSequence(
    const std::vector<const ast::Node*> members) {
  if (members.size() == 1)
    return *members.front();
  return factory().NewSequenceRegExp(ComputeRange(), members);
}

void ScopedNodeFactory::SetToken(const Token& token) {
  end_ = token.end;
}

}  // namespace
#endif

// RegExpParser
RegExpParser::RegExpParser(ParserContext* context,
                           const SourceCodeRange& range,
                           const ParserOptions& options)
    : context_(*context),
      lexer_(new RegExpLexer(context, range, options)),
      options_(options) {}

RegExpParser::~RegExpParser() = default;

const ast::Node& RegExpParser::Parse() {
  return node_factory().NewEmpty(source_code().range());
  // return ParseOr();
}

ast::NodeFactory& RegExpParser::node_factory() const {
  return context_.node_factory();
}

const SourceCode& RegExpParser::source_code() const {
  return lexer_->source_code();
}

#if 0
void RegExpParser::ConsumeToken() {
  last_token_ = PeekToken();
  lexer_.Advance();
}

bool RegExpParser::ConsumeTokenIf(Syntax syntax) {
  if (!CanPeekToken() || PeekToken().syntax != syntax)
    return false;
  ConsumeToken();
  return true;
}

const ast::Node& RegExpParser::ParseOr() {
  ScopedNodeFactory factory(this);
  std::vector<const ast::Node*> members;
  members.push_back(&ParseSequence());
  while (CanPeekToken() && ConsumeTokenIf(Syntax::Or)) {
    if (!CanPeekToken())
      return factory.NewError(ErrorCode::REGEXP_EXPECT_PATTERN);
    members.push_back(&ParseSequence());
  }
  return factory.NewOr(members);
}

const ast::Node& RegExpParser::ParsePrimary() {
  ScopedNodeFactory factory(this);
  if (ConsumeTokenIf(Syntax::AssertBoundary))
    return factory.NewAssertion(ast::RegExpAssertionKind::Boundary);

  if (ConsumeTokenIf(Syntax::AssertBoundaryNot))
    return factory.NewAssertion(ast::RegExpAssertionKind::BoundaryNot);

  if (ConsumeTokenIf(Syntax::AssertEnd))
    return factory.NewAssertion(ast::RegExpAssertionKind::End);

  if (ConsumeTokenIf(Syntax::AssertStart))
    return factory.NewAssertion(ast::RegExpAssertionKind::Start);

  if (ConsumeTokenIf(Syntax::Capture))
    return factory.NewCapture(ParseParenthesis());

  if (ConsumeTokenIf(Syntax::AnyChar))
    return factory.NewAnyChar();

  if (ConsumeTokenIf(Syntax::CharSet))
    return factory.NewCharSet();

  if (ConsumeTokenIf(Syntax::CharSetNot))
    return factory.NewComplementCharSet();

  if (ConsumeTokenIf(Syntax::Literal))
    return factory.NewLiteral();

  if (ConsumeTokenIf(Syntax::Group))
    return ParseParenthesis();

  if (ConsumeTokenIf(Syntax::LookAhead))
    return factory.NewLookAhead(ParseParenthesis());

  if (ConsumeTokenIf(Syntax::LookAheadNot))
    return factory.NewLookAheadNot(ParseParenthesis());

  if (PeekToken().syntax == Syntax::End && !groups_.empty())
    return factory.NewInvalid(ErrorCode::REGEXP_EXPECT_RPAREN);

  if (PeekToken().syntax == Syntax::Invalid) {
    auto& node = factory.NewInvalid(PeekToken().error_code);
    ConsumeToken();
    return node;
  }

  ConsumeToken();
  if (options_.enable_strict_regexp())
    return factory.NewError(ErrorCode::REGEXP_EXPECT_PRIMARY);
  return factory.NewLiteral();
}

const ast::Node& RegExpParser::ParseParenthesis() {
  ScopedNodeFactory factory(this);
  if (ConsumeTokenIf(Syntax::Close)) {
    // In case of /(?:)/
    return factory.NewSequence({});
  }
  groups_.push(last_token_.start);
  auto& pattern = ParseOr();
  DCHECK(!groups_.empty());
  if (!ConsumeTokenIf(Syntax::Close)) {
    factory.AddError(source_code().Slice(groups_.top(), last_token_.end),
                     ErrorCode::REGEXP_EXPECT_RPAREN);
  }
  groups_.pop();
  return pattern;
}

const ast::Node& RegExpParser::ParseRepeat() {
  ScopedNodeFactory factory(this);
  auto& pattern = ParsePrimary();
  if (!CanPeekToken())
    return pattern;
  auto repeat = PeekToken().repeat;
  factory.SetToken(PeekToken());
  if (ConsumeTokenIf(Syntax::GreedyRepeat))
    return factory.NewGreedyRepeat(pattern, repeat);
  if (ConsumeTokenIf(Syntax::LazyRepeat))
    return factory.NewLazyRepeat(pattern, repeat);
  return pattern;
}

bool CanMergeNodes(const ast::Node& pattern1, const ast::Node& pattern2) {
  return pattern1 == ast::SyntaxCode::LiteralRegExp &&
         pattern2 == ast::SyntaxCode::LiteralRegExp;
}

const ast::Node& RegExpParser::ParseSequence() {
  ScopedNodeFactory factory(this);
  std::vector<const ast::Node*> patterns;
  patterns.push_back(&ParseRepeat());
  while (CanPeekToken() && PeekToken().syntax != Syntax::Or) {
    if (PeekToken().syntax == Syntax::End)
      break;
    if (PeekToken().syntax == Syntax::Close && !groups_.empty())
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
#endif

}  // namespace parser
}  // namespace joana

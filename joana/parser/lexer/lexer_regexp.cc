// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>

#include "joana/parser/lexer/lexer.h"

#include "joana/ast/node_factory.h"
#include "joana/ast/regexp.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer_error_codes.h"
#include "joana/parser/public/parse.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/parser/utils/lexer_utils.h"

namespace joana {
namespace parser {

namespace {

constexpr auto kInfinity = ast::RegExpRepeat::kInfinity;

using ErrorCode = Lexer::ErrorCode;

#define FOR_EACH_REGEXP_SYNTAX(V)     \
  V(AssertBoundary, "\\b")            \
  V(AssertBoundaryNot, "\\B")         \
  V(AssertEnd, "$")                   \
  V(AssertStart, "^")                 \
  V(AnyChar, ".")                     \
  V(Capture, "(pattern)")             \
  V(CharSet, "[range]")               \
  V(CharSetNot, "[^range]")           \
  V(Close, "")                        \
  V(End, "/")                         \
  V(Invalid, "")                      \
  V(GreedyRepeat, "pattern{min,max}") \
  V(Group, "(?:pattern)")             \
  V(LazyRepeat, "pattern{min,max}?")  \
  V(Literal, "a")                     \
  V(LookAhead, "(?=pattern)")         \
  V(LookAheadNot, "(?!pattern)")      \
  V(Or, "pattern|pattern")

//
// Syntax
//
enum Syntax {
  None,

#define V(name, string) name,
  FOR_EACH_REGEXP_SYNTAX(V)
#undef V
};

//
// Token
//
struct Token {
  Syntax syntax = Syntax::Invalid;
  int start = 0;
  int end = 0;
  ErrorCode error_code = ErrorCode::None;
  ast::RegExpRepeat repeat;
};

//
// RegExpLexer
//
class RegExpLexer final {
 public:
  RegExpLexer(ParserContext* context,
              CharacterReader* reader,
              const ParserOptions& options);
  ~RegExpLexer();

  int location() const { return reader_.location(); }
  const SourceCode& source_code() const { return reader_.source_code(); }

  void Advance();
  bool CanPeekToken() const;
  const Token& PeekToken() const;
  SourceCodeRange PeekRange() const;

 private:
  void AddError(ErrorCode error_code);
  void HandleCharSet();
  int HandleDigits(int base);
  void HandleRepeat();
  void NextToken();
  void NewLiteral(base::char16 char_code);
  void NewError(ErrorCode error_code);
  void NewRepeat(Syntax op, int min, int max);
  void NewSyntaxChar(Syntax op);

  // CharacterReader helper function
  bool CanPeekChar() const { return reader_.CanPeekChar(); }
  base::char16 ConsumeChar() { return reader_.ConsumeChar(); }
  bool ConsumeCharIf(base::char16 char_code);
  base::char16 PeekChar() const { return reader_.PeekChar(); }

  ParserContext& context_;
  const ParserOptions& options_;
  CharacterReader& reader_;
  Token token_;

  DISALLOW_COPY_AND_ASSIGN(RegExpLexer);
};

RegExpLexer::RegExpLexer(ParserContext* context,
                         CharacterReader* reader,
                         const ParserOptions& options)
    : context_(*context), options_(options), reader_(*reader) {
  NextToken();
}

RegExpLexer::~RegExpLexer() = default;

void RegExpLexer::Advance() {
  NextToken();
}

bool RegExpLexer::CanPeekToken() const {
  return token_.syntax != Syntax::None;
}

void RegExpLexer::AddError(ErrorCode error_code) {
  context_.error_sink().AddError(source_code().Slice(token_.start, location()),
                                 static_cast<int>(error_code));
}

bool RegExpLexer::ConsumeCharIf(base::char16 char_code) {
  return reader_.ConsumeCharIf(char_code);
}

void RegExpLexer::HandleCharSet() {
  token_.syntax = ConsumeCharIf('^') ? Syntax::CharSetNot : Syntax::CharSet;
  // TODO(eval1749): NYI parse char set
  while (!ConsumeCharIf(']')) {
    if (!ConsumeCharIf(kBackslash)) {
      ConsumeChar();
      continue;
    }
    if (!CanPeekChar()) {
      AddError(ErrorCode::REGEXP_EXPECT_RBRACKET);
      break;
    }
    ConsumeChar();
  }
  token_.end = location();
}

int RegExpLexer::HandleDigits(int base) {
  auto number_of_digits = 0;
  auto number = 0;
  while (CanPeekChar() && IsDigitChar(PeekChar(), base)) {
    const auto digit = FromDigitChar(ConsumeChar(), base);
    ++number_of_digits;
    if (number >= kInfinity / base - digit) {
      AddError(ErrorCode::REGEXP_INVALID_NUMBER);
      continue;
    }
    number *= base;
    number += digit;
  }
  if (number_of_digits == 0)
    AddError(ErrorCode::REGEXP_INVALID_NUMBER);
  return number;
}

void RegExpLexer::HandleRepeat() {
  if (!options_.enable_strict_regexp()) {
    if (!CanPeekChar() || !IsDigitChar(PeekChar(), 10))
      return NewLiteral('{');
  }
  const auto min = HandleDigits(10);
  if (ConsumeCharIf(kRightBrace)) {
    if (ConsumeCharIf('?'))
      return NewRepeat(Syntax::LazyRepeat, min, min);
    return NewRepeat(Syntax::GreedyRepeat, min, min);
  }
  if (!ConsumeCharIf(','))
    return NewError(ErrorCode::REGEXP_INVALID_REPEAT);
  const auto max = CanPeekChar() && IsDigitChar(PeekChar(), 10)
                       ? HandleDigits(10)
                       : kInfinity;
  if (!ConsumeCharIf(kRightBrace))
    return NewError(ErrorCode::REGEXP_EXPECT_RBRACE);
  if (ConsumeCharIf('?'))
    return NewRepeat(Syntax::LazyRepeat, min, max);
  return NewRepeat(Syntax::GreedyRepeat, min, max);
}

void RegExpLexer::NextToken() {
  if (!CanPeekChar())
    return NewSyntaxChar(Syntax::End);
  token_.start = location();
  const auto char_code = ConsumeChar();
  switch (char_code) {
    case '$':
      if (options_.enable_strict_regexp())
        return NewSyntaxChar(Syntax::AssertEnd);
      if (!CanPeekChar() || PeekChar() == kRightParenthesis ||
          PeekChar() == '/') {
        return NewSyntaxChar(Syntax::AssertEnd);
      }
      return NewLiteral(char_code);
    case '*':
      if (ConsumeCharIf('?'))
        return NewRepeat(Syntax::LazyRepeat, 0, kInfinity);
      return NewRepeat(Syntax::GreedyRepeat, 0, kInfinity);
    case '+':
      if (ConsumeCharIf('?'))
        return NewRepeat(Syntax::LazyRepeat, 1, kInfinity);
      return NewRepeat(Syntax::GreedyRepeat, 1, kInfinity);
    case '.':
      return NewSyntaxChar(Syntax::AnyChar);
    case '/':
      return NewSyntaxChar(Syntax::End);
    case '?':
      if (ConsumeCharIf('?'))
        return NewRepeat(Syntax::LazyRepeat, 0, 1);
      return NewRepeat(Syntax::GreedyRepeat, 0, 1);
    case '(':
      if (ConsumeCharIf('?')) {
        if (ConsumeCharIf(':'))
          return NewSyntaxChar(Syntax::Group);
        if (ConsumeCharIf('='))
          return NewSyntaxChar(Syntax::LookAhead);
        if (ConsumeCharIf('!'))
          return NewSyntaxChar(Syntax::LookAheadNot);
        if (options_.enable_strict_regexp())
          AddError(ErrorCode::REGEXP_INVALID_GROUPING);
        return NewSyntaxChar(Syntax::Group);
      }
      if (options_.enable_strict_regexp())
        return NewSyntaxChar(Syntax::Group);
      if (!CanPeekChar() || PeekChar() == kRightParenthesis)
        return NewLiteral(char_code);
      return NewSyntaxChar(Syntax::Capture);
    case ')':
      return NewSyntaxChar(Syntax::Close);
    case '[':
      return HandleCharSet();
    case ']':
      return NewLiteral(char_code);
    case '{':
      return HandleRepeat();
    case '}':
      return NewLiteral(char_code);
    case '^':
      if (options_.enable_strict_regexp())
        return NewSyntaxChar(Syntax::AssertStart);
      return NewLiteral(char_code);
    case '|':
      if (options_.enable_strict_regexp())
        return NewSyntaxChar(Syntax::Or);
      if (!CanPeekToken() || PeekChar() == '/' ||
          PeekChar() == kRightParenthesis) {
        return NewLiteral(char_code);
      }
      return NewSyntaxChar(Syntax::Or);
    case '\\':
      // TODO(eval1749): NYI parse backslash
      if (!CanPeekChar()) {
        AddError(ErrorCode::REGEXP_EXPECT_CHAR);
        return NewLiteral(char_code);
      }
      return NewLiteral(ConsumeChar());
  }
  return NewLiteral(char_code);
}

void RegExpLexer::NewError(ErrorCode error_code) {
  AddError(error_code);
  token_.syntax = Syntax::Invalid;
  token_.end = location();
}

void RegExpLexer::NewLiteral(base::char16 char_code) {
  token_.syntax = Syntax::Literal;
  token_.end = location();
}

void RegExpLexer::NewRepeat(Syntax op, int min, int max) {
  token_.syntax = op;
  token_.end = location();
  token_.repeat.min = min;
  token_.repeat.max = max;
}

void RegExpLexer::NewSyntaxChar(Syntax op) {
  token_.syntax = op;
  token_.end = location();
}

const Token& RegExpLexer::PeekToken() const {
  DCHECK(CanPeekToken());
  return token_;
}

//
// RegExpParser
//
class RegExpParser final {
 public:
  RegExpParser(ParserContext* context,
               CharacterReader* reader,
               const ParserOptions& options);
  ~RegExpParser();

  const ast::Node& Run();

 private:
  friend class ScopedNodeFactory;

  ParserContext& context() { return context_; }
  int location() const { return lexer_.location(); }
  ast::NodeFactory& node_factory() { return context_.node_factory(); }
  const SourceCode& source_code() const { return lexer_.source_code(); }

  const ast::Node& ParseOr();
  const ast::Node& ParsePrimary();
  const ast::Node& ParseParenthesis();
  const ast::Node& ParseRepeat();
  const ast::Node& ParseSequence();

  // Helper functions for Lexer
  bool CanPeekToken() const { return lexer_.CanPeekToken(); }
  void ConsumeToken();
  bool ConsumeTokenIf(Syntax syntax);
  const Token& PeekToken() const { return lexer_.PeekToken(); }

  ParserContext& context_;

  // |groups_| holds start offset of left parenthesis for checking matched
  // parenthesis.
  std::stack<int> groups_;

  const ParserOptions& options_;

  // The last consumed token before |ConsumeTokenIf()|.
  Token last_token_;

  RegExpLexer lexer_;

  DISALLOW_COPY_AND_ASSIGN(RegExpParser);
};

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

// RegExpParser
RegExpParser::RegExpParser(ParserContext* context,
                           CharacterReader* reader,
                           const ParserOptions& options)
    : context_(*context), lexer_(context, reader, options), options_(options) {}

RegExpParser::~RegExpParser() = default;

const ast::Node& RegExpParser::Run() {
  return ParseOr();
}

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

}  // namespace

const ast::Node& Lexer::ConsumeRegExp() {
  if (PeekToken() == ast::PunctuatorKind::DivideEqual)
    reader_->MoveBackward();
  return RegExpParser(&context_, reader_.get(), options_).Run();
}

}  // namespace parser
}  // namespace joana

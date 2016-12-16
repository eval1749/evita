// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "joana/parser/parser.h"

#include "base/logging.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/regexp.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/error_sink.h"
#include "joana/public/source_code.h"

namespace joana {
namespace internal {

namespace {

using ErrorCode = Parser::ErrorCode;

const auto kBackslash = '\\';
const auto kLeftBrace = '{';
const auto kRightBrace = '}';
const auto kLeftBracket = '[';
const auto kRightBracket = ']';
const auto kLeftParenthesis = '(';
const auto kRightParenthesis = ')';
constexpr auto kInfinity = std::numeric_limits<int>::max();

int FromDigitChar(base::char16 char_code, int base) {
  DCHECK_GE(base, 2);
  DCHECK_LE(base, 16);
  if (base == 16) {
    if (char_code >= '0' && char_code <= '9')
      return char_code - '0';
    if (char_code >= 'A' && char_code <= 'F')
      return char_code - 'A' + 10;
    if (char_code >= 'a' && char_code <= 'f')
      return char_code - 'a' + 10;
    NOTREACHED() << char_code;
    return 0;
  }
  if (char_code >= '0' && char_code <= '0' + base - 1)
    return char_code - '0';
  NOTREACHED() << char_code;
  return 0;
}

bool IsDigitChar(base::char16 char_code, int base) {
  DCHECK_GE(base, 2);
  DCHECK_LE(base, 16);
  if (base == 16) {
    if (char_code >= '0' && char_code <= '9')
      return true;
    if (char_code >= 'A' && char_code <= 'F')
      return true;
    return char_code >= 'a' && char_code <= 'f';
  }
  return char_code >= '0' && char_code <= '0' + base - 1;
}

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
  Invalid,

#define V(name, string) name,
  FOR_EACH_REGEXP_SYNTAX(V)
#undef V
};

struct Token {
  Syntax syntax = Syntax::Invalid;
  int start = 0;
  int end = 0;
  int min = 0;
  int max = 0;
};

//
// RegExpLexer
//
class RegExpLexer final {
 public:
  RegExpLexer(ast::EditContext* context, Lexer* lexer);
  ~RegExpLexer();

  int location() const { return reader_.location().end(); }
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
  bool ConsumeCharIf(base::char16 char_code) {
    return reader_.ConsumeCharIf(char_code);
  }
  base::char16 PeekChar() const { return reader_.PeekChar(); }

  ast::EditContext& context_;
  Lexer& reader_;
  Token token_;

  DISALLOW_COPY_AND_ASSIGN(RegExpLexer);
};

RegExpLexer::RegExpLexer(ast::EditContext* context, Lexer* reader)
    : context_(*context), reader_(*reader) {
  NextToken();
}

void RegExpLexer::Advance() {
  DCHECK(CanPeekChar());
  NextToken();
}

bool RegExpLexer::CanPeekToken() const {
  return token_.syntax != Syntax::Invalid;
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
      AddError(ErrorCode::ERROR_REGEXP_EXPECT_RBRACKET);
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
      AddError(ErrorCode::ERROR_REGEXP_INVALID_NUMBER);
      continue;
    }
    number *= base;
    number += digit;
  }
  if (number_of_digits == 0)
    AddError(ErrorCode::ERROR_REGEXP_INVALID_NUMBER);
  return number;
}

void RegExpLexer::HandleRepeat() {
  const auto min = HandleDigits(10);
  if (ConsumeCharIf(kRightBrace)) {
    if (ConsumeCharIf('?'))
      return NewRepeat(Syntax::LazyRepeat, min, min);
    return NewRepeat(Syntax::GreedyRepeat, min, min);
  }
  if (!ConsumeCharIf(','))
    return NewError(ErrorCode::ERROR_REGEXP_INVALID_REPEAT);
  const auto max = HandleDigits(10);
  if (!ConsumeCharIf(kRightBrace))
    return NewError(ErrorCode::ERROR_REGEXP_EXPECT_RBRACE);
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
      return CanPeekChar() ? NewLiteral(char_code)
                           : NewSyntaxChar(Syntax::AssertEnd);
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
      if (!ConsumeCharIf('?'))
        return NewSyntaxChar(Syntax::Capture);
      if (ConsumeCharIf(':'))
        return NewSyntaxChar(Syntax::Group);
      if (ConsumeCharIf('='))
        return NewSyntaxChar(Syntax::LookAhead);
      if (ConsumeCharIf('!'))
        return NewSyntaxChar(Syntax::LookAheadNot);
      AddError(ErrorCode::ERROR_REGEXP_INVALID_GROUPING);
      return NewSyntaxChar(Syntax::Group);
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
      return CanPeekToken() ? NewLiteral(char_code)
                            : NewSyntaxChar(Syntax::AssertStart);
    case '|':
      return NewSyntaxChar(Syntax::Or);
    case '\\':
      // TODO(eval1749): NYI parse backslash
      if (!CanPeekChar()) {
        AddError(ErrorCode::ERROR_REGEXP_EXPECT_CHAR);
        return NewLiteral(char_code);
      }
      return NewLiteral(ConsumeChar());
  }
  return NewLiteral(char_code);
}

void RegExpLexer::NewLiteral(base::char16 char_code) {
  token_.syntax = Syntax::Literal;
  token_.end = location();
}

void RegExpLexer::NewRepeat(Syntax op, int min, int max) {
  token_.syntax = op;
  token_.end = location();
  token_.min = min;
  token_.max = max;
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
  RegExpParser(ast::EditContext* context, Lexer* lexer);
  ~RegExpParser();

  ast::RegExp& Run();

 private:
  friend class ScopedNodeFactory;

  ast::EditContext& context() { return context_; }
  int location() const { return lexer_.location(); }
  ast::NodeFactory& node_factory() { return context_.node_factory(); }
  const SourceCode& source_code() const { return lexer_.source_code(); }

  ast::RegExp& ParseOr();
  ast::RegExp& ParsePrimary();
  ast::RegExp& ParseParenthesis();
  ast::RegExp& ParseRepeat();
  ast::RegExp& ParseSequence();

  // Helper functions for Lexer
  bool CanPeekToken() const { return lexer_.CanPeekToken(); }
  bool ConsumeTokenIf(Syntax syntax);
  const Token& PeekToken() const { return lexer_.PeekToken(); }

  ast::EditContext& context_;
  RegExpLexer lexer_;
  int nesting_ = 0;

  DISALLOW_COPY_AND_ASSIGN(RegExpParser);
};

//
// ScopedNodeFactory
//
class ScopedNodeFactory final {
 public:
  ScopedNodeFactory(RegExpParser* parser);
  ~ScopedNodeFactory() = default;

  void AddError(ErrorCode error_code);
  ast::RegExp& NewAnyChar();
  ast::RegExp& NewAssertion(ast::RegExpAssertionKind kind);
  ast::RegExp& NewCapture(ast::RegExp& expression);
  ast::RegExp& NewCharSet();
  ast::RegExp& NewComplementCharSet();
  ast::RegExp& NewError(ErrorCode error_code);
  ast::RegExp& NewGreedyRepeat(ast::RegExp& expression);
  ast::RegExp& NewLazyRepeat(ast::RegExp& expression);
  ast::RegExp& NewLookAhead(ast::RegExp& expression);
  ast::RegExp& NewLookAheadNot(ast::RegExp& expression);
  ast::RegExp& NewLiteral();
  ast::RegExp& NewOr(const std::vector<ast::RegExp*> members);
  ast::RegExp& NewSequence(const std::vector<ast::RegExp*> members);

 private:
  ast::NodeFactory& factory() { return parser_.node_factory(); }

  SourceCodeRange ComputeRange() const;

  RegExpParser& parser_;
  const int start_;

  DISALLOW_COPY_AND_ASSIGN(ScopedNodeFactory);
};

ScopedNodeFactory::ScopedNodeFactory(RegExpParser* parser)
    : parser_(*parser), start_(parser->location()) {}

void ScopedNodeFactory::AddError(ErrorCode error_code) {
  parser_.context().error_sink().AddError(ComputeRange(),
                                          static_cast<int>(error_code));
}

SourceCodeRange ScopedNodeFactory::ComputeRange() const {
  return parser_.source_code().Slice(start_, parser_.lexer_.location());
}

ast::RegExp& ScopedNodeFactory::NewAnyChar() {
  return factory().NewAnyCharRegExp(ComputeRange());
}

ast::RegExp& ScopedNodeFactory::NewAssertion(ast::RegExpAssertionKind kind) {
  return factory().NewAssertionRegExp(ComputeRange(), kind);
}

ast::RegExp& ScopedNodeFactory::NewCapture(ast::RegExp& expression) {
  return factory().NewCaptureRegExp(ComputeRange(), expression);
}

ast::RegExp& ScopedNodeFactory::NewCharSet() {
  return factory().NewCharSetRegExp(ComputeRange());
}

ast::RegExp& ScopedNodeFactory::NewComplementCharSet() {
  return factory().NewComplementCharSetRegExp(ComputeRange());
}

ast::RegExp& ScopedNodeFactory::NewError(ErrorCode error_code) {
  AddError(error_code);
  return factory().NewInvalidRegExp(ComputeRange(),
                                    static_cast<int>(error_code));
}

ast::RegExp& ScopedNodeFactory::NewGreedyRepeat(ast::RegExp& expression) {
  return factory().NewGreedyRepeatRegExp(ComputeRange(), expression);
}

ast::RegExp& ScopedNodeFactory::NewLazyRepeat(ast::RegExp& expression) {
  return factory().NewLazyRepeatRegExp(ComputeRange(), expression);
}

ast::RegExp& ScopedNodeFactory::NewLookAhead(ast::RegExp& expression) {
  return factory().NewLookAheadRegExp(ComputeRange(), expression);
}

ast::RegExp& ScopedNodeFactory::NewLookAheadNot(ast::RegExp& expression) {
  return factory().NewLookAheadNotRegExp(ComputeRange(), expression);
}

ast::RegExp& ScopedNodeFactory::NewLiteral() {
  return factory().NewLiteralRegExp(ComputeRange());
}

ast::RegExp& ScopedNodeFactory::NewOr(const std::vector<ast::RegExp*> members) {
  if (members.size() == 1)
    return *members.front();
  if (members.size() >= 2)
    return factory().NewOrRegExp(ComputeRange(), members);
  return NewError(ErrorCode::ERROR_REGEXP_INVALID_OR);
}

ast::RegExp& ScopedNodeFactory::NewSequence(
    const std::vector<ast::RegExp*> members) {
  if (members.size() == 1)
    return *members.front();
  if (members.size() >= 2)
    return factory().NewSequenceRegExp(ComputeRange(), members);
  return NewError(ErrorCode::ERROR_REGEXP_INVALID_SEQUENCE);
}

// RegExpParser
RegExpParser::RegExpParser(ast::EditContext* context, Lexer* reader)
    : context_(*context), lexer_(context, reader) {}

RegExpParser::~RegExpParser() = default;

ast::RegExp& RegExpParser::Run() {
  return ParseOr();
}

ast::RegExp& RegExpParser::ParseOr() {
  ScopedNodeFactory factory(this);
  std::vector<ast::RegExp*> members;
  members.push_back(&ParseSequence());
  while (CanPeekToken() && ConsumeTokenIf(Syntax::Or)) {
    if (!CanPeekToken())
      return factory.NewError(ErrorCode::ERROR_REGEXP_EXPECT_PATTERN);
    members.push_back(&ParseSequence());
  }
  return factory.NewOr(members);
}

ast::RegExp& RegExpParser::ParsePrimary() {
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

  return factory.NewError(ErrorCode::ERROR_REGEXP_EXPECT_PRIMARY);
}

ast::RegExp& RegExpParser::ParseParenthesis() {
  ScopedNodeFactory factory(this);
  ++nesting_;
  auto& expression = ParseOr();
  DCHECK_GE(nesting_, 1);
  --nesting_;
  if (!ConsumeTokenIf(Syntax::Close))
    factory.AddError(ErrorCode::ERROR_REGEXP_EXPECT_RPAREN);
  return expression;
}

ast::RegExp& RegExpParser::ParseRepeat() {
  ScopedNodeFactory factory(this);
  auto& expression = ParsePrimary();
  if (!CanPeekToken())
    return expression;
  if (ConsumeTokenIf(Syntax::GreedyRepeat))
    return factory.NewGreedyRepeat(expression);
  if (ConsumeTokenIf(Syntax::LazyRepeat))
    return factory.NewLazyRepeat(expression);
  return expression;
}

ast::RegExp& RegExpParser::ParseSequence() {
  ScopedNodeFactory factory(this);
  std::vector<ast::RegExp*> members;
  members.push_back(&ParseRepeat());
  while (CanPeekToken() && PeekToken().syntax != Syntax::Or) {
    if (PeekToken().syntax == Syntax::Close) {
      if (nesting_ > 0)
        break;
      lexer_.Advance();
      factory.AddError(ErrorCode::ERROR_REGEXP_UNEXPECT_RPAREN);
      continue;
    }
    members.push_back(&ParseRepeat());
  }
  return factory.NewSequence(members);
}

}  // namespace

ast::RegExp& Parser::ParseRegExp() {
  return RegExpParser(context_, lexer_.get()).Run();
}

}  // namespace internal
}  // namespace joana

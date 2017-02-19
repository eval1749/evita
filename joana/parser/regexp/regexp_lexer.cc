// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/regexp/regexp_lexer.h"

#include "joana/ast/regexp.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/parser/regexp/regexp_error_codes.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/parser/utils/lexer_utils.h"

namespace joana {
namespace parser {

namespace {

bool IsRepeatChar(base::char16 char_code) {
  return char_code == '*' || char_code == '+' || char_code == '?' ||
         char_code == kLeftBrace;
}

}  // namespace

//
// RegExpLiexer
//
RegExpLexer::RegExpLexer(ParserContext* context,
                         const SourceCodeRange& range,
                         const ParserOptions& options)
    : context_(*context),
      options_(options),
      range_(range),
      reader_(new CharacterReader(range)) {
  current_token_ = NextToken();
}

RegExpLexer::~RegExpLexer() = default;

// public
int RegExpLexer::location() const {
  return reader_->location();
}

const SourceCode& RegExpLexer::source_code() const {
  return range_.source_code();
}

bool RegExpLexer::CanPeekToken() const {
  return current_token_ != nullptr;
}

const ast::Node& RegExpLexer::ConsumeToken() {
  DCHECK(current_token_);
  const auto* token = current_token_;
  current_token_ = NextToken();
  return *token;
}

const ast::Node& RegExpLexer::PeekToken() const {
  DCHECK(CanPeekToken());
  return *current_token_;
}

// private
ast::NodeFactory& RegExpLexer::node_factory() const {
  return context_.node_factory();
}

const ast::Node& RegExpLexer::HandleCharSet() {
  const auto is_complement = ConsumeCharIf('^');
  // TODO(eval1749): NYI parse char set
  while (CanPeekChar() && PeekChar() != kRightBracket) {
    if (!ConsumeCharIf(kBackslash)) {
      ConsumeChar();
      continue;
    }
    if (!CanPeekChar())
      break;
    ConsumeChar();
  }
  if (!CanPeekChar() || !ConsumeCharIf(kRightBracket))
    AddError(RegExpErrorCode::REGEXP_EXPECT_RBRACKET);
  return is_complement
             ? node_factory().NewComplementCharSetRegExp(MakeTokenRange())
             : node_factory().NewCharSetRegExp(MakeTokenRange());
}

int RegExpLexer::HandleDigits(int base) {
  auto number_of_digits = 0;
  auto number = 0;
  while (CanPeekChar() && IsDigitChar(PeekChar(), base)) {
    const auto digit = FromDigitChar(ConsumeChar(), base);
    ++number_of_digits;
    if (number >= ast::kRegExpInfinity / base - digit) {
      AddError(RegExpErrorCode::REGEXP_INVALID_NUMBER);
      continue;
    }
    number *= base;
    number += digit;
  }
  if (number_of_digits == 0)
    AddError(RegExpErrorCode::REGEXP_INVALID_NUMBER);
  return number;
}

const ast::Node& RegExpLexer::HandleRepeat() {
  if (!options_.enable_strict_regexp()) {
    if (!CanPeekChar() || !IsDigitChar(PeekChar(), 10))
      return NewLiteral();
  }
  const auto min = HandleDigits(10);
  if (ConsumeCharIf(kRightBrace)) {
    if (ConsumeCharIf('?'))
      return NewRepeat(ast::RegExpRepeatMethod::Lazy, min, min);
    return NewRepeat(ast::RegExpRepeatMethod::Greedy, min, min);
  }
  if (!ConsumeCharIf(','))
    AddError(RegExpErrorCode::REGEXP_INVALID_REPEAT);
  const auto max = CanPeekChar() && IsDigitChar(PeekChar(), 10)
                       ? HandleDigits(10)
                       : ast::kRegExpInfinity;
  if (!ConsumeCharIf(kRightBrace))
    AddError(RegExpErrorCode::REGEXP_EXPECT_RBRACE);
  if (ConsumeCharIf('?'))
    return NewRepeat(ast::RegExpRepeatMethod::Lazy, min, max);
  return NewRepeat(ast::RegExpRepeatMethod::Greedy, min, max);
}

bool RegExpLexer::IsSyntaxChar(base::char16 char_code) const {
  if (char_code == '$' || char_code == '*' || char_code == '+' ||
      char_code == '?' || char_code == '|' || char_code == kLeftParenthesis ||
      char_code == kLeftBrace || char_code == kLeftBracket ||
      char_code == kBackslash) {
    return true;
  }
  if (group_ > 0 && char_code == kRightParenthesis)
    return true;
  if (!options_.enable_strict_regexp())
    return false;
  return char_code == '^' || char_code == kRightBrace ||
         char_code == kRightBracket;
}

SourceCodeRange RegExpLexer::MakeTokenRange() const {
  return source_code().Slice(token_start_, reader_->location());
}

const ast::Node* RegExpLexer::NextToken() {
  if (!CanPeekChar())
    return nullptr;
  token_start_ = location();
  const auto char_code = ConsumeChar();
  switch (char_code) {
    case '$':
      if (!CanPeekChar() || options_.enable_strict_regexp())
        return &NewAssertion(ast::RegExpAssertionKind::End);
      if (options_.enable_strict_regexp()) {
        AddError(RegExpErrorCode::REGEXP_INVALID_DOLLAR);
        return &NewAssertion(ast::RegExpAssertionKind::End);
      }
      break;
    case '*':
      if (!options_.enable_strict_regexp() && token_start_ == range_.start())
        break;
      if (ConsumeCharIf('?'))
        return &NewRepeat(ast::RegExpRepeatMethod::Lazy, 0,
                          ast::kRegExpInfinity);
      return &NewRepeat(ast::RegExpRepeatMethod::Greedy, 0,
                        ast::kRegExpInfinity);
    case '+':
      if (!options_.enable_strict_regexp() && token_start_ == range_.start())
        break;
      if (ConsumeCharIf('?'))
        return &NewRepeat(ast::RegExpRepeatMethod::Lazy, 1,
                          ast::kRegExpInfinity);
      return &NewRepeat(ast::RegExpRepeatMethod::Greedy, 1,
                        ast::kRegExpInfinity);
    case '.':
      return &node_factory().NewAnyCharRegExp(MakeTokenRange());
    case '?':
      if (ConsumeCharIf('?'))
        return &NewRepeat(ast::RegExpRepeatMethod::Lazy, 0, 1);
      return &NewRepeat(ast::RegExpRepeatMethod::Greedy, 0, 1);
    case '(':
      ++group_;
      if (ConsumeCharIf('?')) {
        if (ConsumeCharIf(':'))
          return &NewSyntaxChar(ast::TokenKind::Colon);
        if (ConsumeCharIf('='))
          return &NewSyntaxChar(ast::TokenKind::Equal);
        if (ConsumeCharIf('!'))
          return &NewSyntaxChar(ast::TokenKind::LogicalNot);
        if (options_.enable_strict_regexp())
          AddError(RegExpErrorCode::REGEXP_INVALID_GROUPING);
        return &NewSyntaxChar(ast::TokenKind::Colon);
      }
      if (options_.enable_strict_regexp())
        return &NewSyntaxChar(ast::TokenKind::Colon);
      if (!CanPeekChar() || PeekChar() == kRightParenthesis)
        return &NewLiteral();
      return &NewSyntaxChar(ast::TokenKind::LeftParenthesis);
    case ')':
      if (group_ > 0) {
        --group_;
        return &NewSyntaxChar(ast::TokenKind::RightParenthesis);
      }
      if (options_.enable_strict_regexp())
        return &NewSyntaxChar(ast::TokenKind::RightParenthesis);
      break;
    case '[':
      return &HandleCharSet();
    case '{':
      if (!options_.enable_strict_regexp() && token_start_ == range_.start())
        break;
      return &HandleRepeat();
    case '^':
      if (token_start_ == range_.start())
        return &NewAssertion(ast::RegExpAssertionKind::Start);
      if (options_.enable_strict_regexp())
        return &NewAssertion(ast::RegExpAssertionKind::Start);
      break;
    case '|':
      if (options_.enable_strict_regexp())
        return &NewSyntaxChar(ast::TokenKind::BitOr);
      return &NewSyntaxChar(ast::TokenKind::BitOr);
    case '\\':
      // TODO(eval1749): NYI parse backslash
      if (!CanPeekChar())
        AddError(RegExpErrorCode::REGEXP_EXPECT_CHAR);
      if (ConsumeCharIf('b'))
        return &NewAssertion(ast::RegExpAssertionKind::Boundary);
      if (ConsumeCharIf('B'))
        return &NewAssertion(ast::RegExpAssertionKind::BoundaryNot);
      ConsumeChar();
      break;
  }
  while (CanPeekChar() && !IsSyntaxChar(PeekChar()))
    ConsumeChar();
  if (!CanPeekChar())
    return &NewLiteral();
  if (reader_->location() - token_start_ > 1 && IsRepeatChar(PeekChar()))
    reader_->MoveBackward();
  return &NewLiteral();
}

// ast::NodeFactory helper functions
const ast::Node& RegExpLexer::NewAssertion(ast::RegExpAssertionKind kind) {
  return node_factory().NewAssertionRegExp(MakeTokenRange(), kind);
}

const ast::Node& RegExpLexer::NewLiteral() {
  DCHECK_LT(token_start_, reader_->location());
  return node_factory().NewLiteralRegExp(MakeTokenRange());
}

const ast::Node& RegExpLexer::NewRepeat(ast::RegExpRepeatMethod method,
                                        int min,
                                        int max) {
  return node_factory().NewRegExpRepeat(MakeTokenRange(), method, min, max);
}

const ast::Node& RegExpLexer::NewSyntaxChar(ast::TokenKind op) {
  return node_factory().NewPunctuator(MakeTokenRange(), op);
}

// CharacterReader helper function
bool RegExpLexer::CanPeekChar() const {
  return reader_->CanPeekChar();
}

base::char16 RegExpLexer::ConsumeChar() {
  return reader_->ConsumeChar();
}

bool RegExpLexer::ConsumeCharIf(base::char16 char_code) {
  return reader_->ConsumeCharIf(char_code);
}
base::char16 RegExpLexer::PeekChar() const {
  return reader_->PeekChar();
}

void RegExpLexer::AddError(RegExpErrorCode error_code) {
  context_.error_sink().AddError(MakeTokenRange(), error_code);
}

}  // namespace parser
}  // namespace joana

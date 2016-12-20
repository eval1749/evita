// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <cmath>
#include <limits>
#include <vector>

#include "joana/parser/lexer/lexer.h"

#include "joana/parser/lexer/character_reader.h"
#include "joana/parser/lexer/lexer_error_codes.h"
#include "joana/parser/lexer/lexer_utils.h"
#include "joana/parser/public/parse.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/error_codes.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/error_sink.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_range.h"

namespace joana {
namespace internal {

namespace {

const auto kMaxIntegerPart = static_cast<uint64_t>(1) << 53;

}  // namespace

Lexer::Lexer(ast::EditContext* context,
             const SourceCodeRange& range,
             const ParserOptions& options)
    : context_(*context),
      options_(options),
      range_(range),
      reader_(new CharacterReader(range)),
      token_start_(range.start()) {
  current_token_ = NextToken();
}

Lexer::~Lexer() = default;

SourceCodeRange Lexer::location() const {
  return source_code().Slice(reader_->location(), reader_->location());
}

ast::NodeFactory& Lexer::node_factory() const {
  return context_.node_factory();
}

const SourceCode& Lexer::source_code() const {
  return reader_->source_code();
}

void Lexer::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  context_.error_sink().AddError(range, static_cast<int>(error_code));
}

void Lexer::AddError(ErrorCode error_code) {
  AddError(MakeTokenRange(), error_code);
}

void Lexer::Advance() {
  DCHECK(current_token_);
  current_token_ = NextToken();
}

bool Lexer::CanPeekChar() const {
  return reader_->CanPeekChar();
}

base::char16 Lexer::ConsumeChar() {
  return reader_->ConsumeChar();
}

bool Lexer::ConsumeCharIf(base::char16 char_code) {
  return reader_->ConsumeCharIf(char_code);
}

base::char16 Lexer::PeekChar() const {
  return reader_->PeekChar();
}

ast::Token& Lexer::PeekToken() const {
  DCHECK(current_token_);
  return *current_token_;
}

ast::Token& Lexer::HandleBlockComment() {
  auto is_after_asterisk = false;
  while (reader_->CanPeekChar()) {
    if (is_after_asterisk && ConsumeCharIf('/'))
      return node_factory().NewComment(MakeTokenRange());
    if (IsLineTerminator(PeekChar()))
      is_separated_by_newline_ = true;
    is_after_asterisk = ConsumeChar() == '*';
  }
  return NewInvalid(ErrorCode::BLOCK_COMMENT_NOT_CLOSED);
}

ast::Token& Lexer::HandleCharacter() {
  token_start_ = reader_->location();
  switch (PeekChar()) {
    case ' ':
    case 0x0009:  // t TAB
    case 0x000A:  // n LF
    case 0x000B:  // v VT
    case 0x000C:  // f FF
    case 0x000D:  // r CR
    case '!':
      ConsumeChar();
      if (ConsumeCharIf('=')) {
        if (ConsumeCharIf('='))
          return NewPunctuator(ast::PunctuatorKind::NotEqualEqual);
        return NewPunctuator(ast::PunctuatorKind::NotEqual);
      }
      return NewPunctuator(ast::PunctuatorKind::LogicalNot);
    case '"':
      return HandleStringLiteral();
    case '#':
      goto invalid_character;
    case '$':
      return HandleName();
    case '%':
      return HandleOperator(ast::PunctuatorKind::Modulo,
                            ast::PunctuatorKind::Invalid,
                            ast::PunctuatorKind::ModuloEqual);
    case '&':
      return HandleOperator(ast::PunctuatorKind::BitAnd,
                            ast::PunctuatorKind::LogicalAnd,
                            ast::PunctuatorKind::BitAndEqual);
    case '\'':
      return HandleStringLiteral();
    case '(':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::LeftParenthesis);
    case ')':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::RightParenthesis);
    case '*':
      ConsumeChar();
      if (ConsumeCharIf('='))
        return NewPunctuator(ast::PunctuatorKind::TimesEqual);
      if (ConsumeCharIf('*')) {
        if (ConsumeCharIf('='))
          return NewPunctuator(ast::PunctuatorKind::TimesTimesEqual);
        return NewPunctuator(ast::PunctuatorKind::TimesTimes);
      }
      return NewPunctuator(ast::PunctuatorKind::Times);
    case '+':
      return HandleOperator(ast::PunctuatorKind::Plus,
                            ast::PunctuatorKind::PlusPlus,
                            ast::PunctuatorKind::PlusEqual);
    case ',':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::Comma);
    case '-':
      return HandleOperator(ast::PunctuatorKind::Minus,
                            ast::PunctuatorKind::MinusMinus,
                            ast::PunctuatorKind::MinusEqual);
    case '.':
      ConsumeChar();
      if (ConsumeCharIf('.')) {
        if (ConsumeCharIf('.'))
          return NewPunctuator(ast::PunctuatorKind::DotDotDot);
        return NewError(ErrorCode::PUNCTUATOR_DOT_DOT);
      }
      if (CanPeekChar() && IsDigitChar(PeekChar(), 10)) {
        reader_->MoveBackward();
        return HandleDecimalAfterDot(0, 0);
      }
      return NewPunctuator(ast::PunctuatorKind::Dot);
    case '/':
      ConsumeChar();
      if (ConsumeCharIf('='))
        return NewPunctuator(ast::PunctuatorKind::DivideEqual);
      if (ConsumeCharIf('*'))
        return HandleBlockComment();
      if (ConsumeCharIf('/'))
        return HandleLineComment();
      return NewPunctuator(ast::PunctuatorKind::Divide);
    case '0':
      ConsumeChar();
      return HandleDigitZero();
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return HandleDecimal();
    case ':':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::Colon);
    case ';':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::Semicolon);
    case '<':
      ConsumeChar();
      if (ConsumeCharIf('='))
        return NewPunctuator(ast::PunctuatorKind::LessThanOrEqual);
      if (ConsumeCharIf('<')) {
        if (ConsumeCharIf('='))
          return NewPunctuator(ast::PunctuatorKind::LeftShiftEqual);
        return NewPunctuator(ast::PunctuatorKind::LeftShift);
      }
      return NewPunctuator(ast::PunctuatorKind::LessThan);
    case '=':
      ConsumeChar();
      if (ConsumeCharIf('=')) {
        if (ConsumeCharIf('='))
          return NewPunctuator(ast::PunctuatorKind::EqualEqualEqual);
        return NewPunctuator(ast::PunctuatorKind::EqualEqual);
      }
      if (ConsumeCharIf('>'))
        return NewPunctuator(ast::PunctuatorKind::Arrow);
      return NewPunctuator(ast::PunctuatorKind::Equal);
    case '>':
      ConsumeChar();
      if (ConsumeCharIf('='))
        return NewPunctuator(ast::PunctuatorKind::GreaterThanOrEqual);
      if (ConsumeCharIf('>')) {
        if (ConsumeCharIf('='))
          return NewPunctuator(ast::PunctuatorKind::RightShiftEqual);
        if (ConsumeCharIf('>')) {
          if (ConsumeCharIf('='))
            return NewPunctuator(ast::PunctuatorKind::UnsignedRightShiftEqual);
          return NewPunctuator(ast::PunctuatorKind::UnsignedRightShift);
        }
        return NewPunctuator(ast::PunctuatorKind::RightShift);
      }
      return NewPunctuator(ast::PunctuatorKind::GreaterThan);
    case '?':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::Question);
    case '`':
      // TODO(eval1749): NYI: template token
      return HandleStringLiteral();
    case '[':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::LeftBracket);
    case ']':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::RightBracket);
    case '_':
      return HandleName();
    case '^':
      return HandleOperator(ast::PunctuatorKind::BitXor,
                            ast::PunctuatorKind::Invalid,
                            ast::PunctuatorKind::BitXorEqual);
    case '~':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::BitNot);
    case '{':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::LeftBrace);
    case '}':
      ConsumeChar();
      return NewPunctuator(ast::PunctuatorKind::RightBrace);
    case '|':
      return HandleOperator(ast::PunctuatorKind::BitOr,
                            ast::PunctuatorKind::LogicalOr,
                            ast::PunctuatorKind::BitOrEqual);
    invalid_character:
    default:
      if (IsIdentifierStart(PeekChar()))
        return HandleName();
      ConsumeChar();
      return NewInvalid(ErrorCode::CHARACTER_INVALID);
  }
}

ast::Token& Lexer::HandleDecimal() {
  uint64_t integer_part = FromDigitChar(ConsumeChar(), 10);
  auto integer_scale = 0;
  while (reader_->CanPeekChar()) {
    if (!IsDigitChar(PeekChar(), 10))
      break;
    const auto digit = FromDigitChar(ConsumeChar(), 10);
    if (integer_part > std::numeric_limits<uint64_t>::max() / 10 - digit) {
      ++integer_scale;
      continue;
    }
    integer_part *= 10;
    integer_part += digit;
  }
  return HandleDecimalAfterDot(integer_part, integer_scale);
}

ast::Token& Lexer::HandleDecimalAfterDot(uint64_t integer_part,
                                         int integer_scale) {
  auto digits_part = integer_part;
  auto digits_scale = integer_scale;
  if (ConsumeCharIf('.')) {
    while (reader_->CanPeekChar()) {
      if (!IsDigitChar(PeekChar(), 10))
        break;
      const auto digit = FromDigitChar(ConsumeChar(), 10);
      if (digits_part > kMaxIntegerPart) {
        // Since we've already had number of digits more than precision, just
        // ignore digit digits_part decimal point.
        continue;
      }
      --digits_scale;
      digits_part *= 10;
      digits_part += digit;
    }
  }
  uint64_t exponent_part = 0;
  auto exponent_sign = 0;
  if (ConsumeCharIf('E') || ConsumeCharIf('e')) {
    if (ConsumeCharIf('+'))
      exponent_sign = 1;
    else if (ConsumeCharIf('-'))
      exponent_sign = -1;
    while (reader_->CanPeekChar()) {
      if (!IsDigitChar(PeekChar(), 10))
        break;
      const auto digit = FromDigitChar(ConsumeChar(), 10);
      if (exponent_part > kMaxIntegerPart)
        continue;
      exponent_part *= 10;
      exponent_part += digit;
    }
  }
  auto number_of_invalid = 0;
  while (CanPeekChar() && IsIdentifierPart(PeekChar())) {
    ++number_of_invalid;
    ConsumeChar();
    if (number_of_invalid > 1)
      continue;
    AddError(ErrorCode::NUMERIC_LITERAL_DECIMAL_BAD_DIGIT);
  }
  if (number_of_invalid > 0)
    return NewInvalid(ErrorCode::NUMERIC_LITERAL_DECIMAL_BAD_DIGIT);
  const auto value =
      static_cast<double>(digits_part) * std::pow(10.0, digits_scale);
  const auto exponent = exponent_part * exponent_sign;
  return node_factory().NewNumericLiteral(MakeTokenRange(),
                                          value * std::pow(10.0, exponent));
}

ast::Token& Lexer::HandleDigitZero() {
  if (!CanPeekChar())
    return node_factory().NewNumericLiteral(MakeTokenRange(), 0);
  switch (PeekChar()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      // Continue fetching octal digits
      HandleInteger(8);
      return NewError(ErrorCode::NUMERIC_LITERAL_INTEGER_OCTAL);
    case '8':
    case '9':
      return HandleDecimal();
    case 'b':
    case 'B':
      ConsumeChar();
      return HandleInteger(2);
    case 'o':
    case 'O':
      ConsumeChar();
      return HandleInteger(8);
    case 'x':
    case 'X':
      ConsumeChar();
      return HandleInteger(16);
  }
  return HandleDecimalAfterDot(0, 0);
}

ast::Token& Lexer::HandleInteger(int base) {
  DCHECK(base == 2 || base == 8 || base == 16) << base;
  uint64_t accumulator = 0;
  const auto kMaxInteger = static_cast<uint64_t>(1) << 53;
  auto number_of_digits = 0;
  auto is_overflow = false;
  while (reader_->CanPeekChar() && IsDigitChar(PeekChar(), base)) {
    const auto digit = FromDigitChar(ConsumeChar(), base);
    if (digit != 0 && accumulator > kMaxInteger / base) {
      if (!is_overflow)
        AddError(ErrorCode::NUMERIC_LITERAL_INTEGER_OVERFLOW);
      is_overflow = true;
    } else {
      accumulator *= base;
      accumulator += digit;
    }
    ++number_of_digits;
  }

  if (reader_->CanPeekChar() && IsIdentifierPart(PeekChar())) {
    ConsumeChar();
    AddError(ErrorCode::NUMERIC_LITERAL_INTEGER_BAD_DIGIT);
    while (reader_->CanPeekChar() && IsIdentifierPart(PeekChar()))
      ConsumeChar();
    return NewInvalid(ErrorCode::NUMERIC_LITERAL_INTEGER_BAD_DIGIT);
  }
  if (number_of_digits == 0)
    return NewError(ErrorCode::NUMERIC_LITERAL_INTEGER_NO_DIGITS);
  return node_factory().NewNumericLiteral(MakeTokenRange(), accumulator);
}

ast::Token& Lexer::HandleLineComment() {
  while (reader_->CanPeekChar()) {
    if (IsLineTerminator(ConsumeChar())) {
      is_separated_by_newline_ = true;
      break;
    }
  }
  return node_factory().NewComment(MakeTokenRange());
}

ast::Token& Lexer::HandleName() {
  while (reader_->CanPeekChar()) {
    if (!IsIdentifierPart(PeekChar()))
      break;
    ConsumeChar();
  }
  return node_factory().NewName(MakeTokenRange());
}

// Handle op, op op, op '=' pattern.
ast::Token& Lexer::HandleOperator(ast::PunctuatorKind one,
                                  ast::PunctuatorKind two,
                                  ast::PunctuatorKind equal) {
  const auto char_code = ConsumeChar();
  if (ConsumeCharIf('='))
    return NewPunctuator(equal);
  if (two != ast::PunctuatorKind::Invalid && ConsumeCharIf(char_code))
    return NewPunctuator(two);
  return NewPunctuator(one);
}

ast::Token& Lexer::HandleStringLiteral() {
  std::vector<base::char16> characters;
  enum class State {
    Backslash,
    BackslashCr,
    BackslashU,
    BackslashU1,
    BackslashU2,
    BackslashU3,
    BackslashUB,
    BackslashUBx,
    BackslashX,
    BackslashX1,
    Normal,
  } state = State::Normal;
  const auto delimiter = ConsumeChar();
  auto accumulator = 0;
  auto backslash_start = 0;
  while (reader_->CanPeekChar()) {
    switch (state) {
      case State::Normal:
        if (ConsumeCharIf(delimiter)) {
          return node_factory().NewStringLiteral(
              MakeTokenRange(),
              base::StringPiece16(characters.data(), characters.size()));
        }
        if (PeekChar() == '\\') {
          backslash_start = reader_->location();
          state = State::Backslash;
          break;
        }
        if (delimiter != '`' && IsLineTerminator(PeekChar()))
          AddError(RangeFrom(token_start_), ErrorCode::STRING_LITERAL_NEWLINE);
        characters.push_back(PeekChar());
        break;
      case State::Backslash:
        switch (PeekChar()) {
          case '\'':
          case '"':
          case '\\':
            state = State::Normal;
            characters.push_back(PeekChar());
            break;
          case '0':
            ConsumeChar();
            if (!CanPeekChar())
              return NewError(ErrorCode::STRING_LITERAL_NOT_CLOSED);
            if (options_.enable_strict_backslash &&
                IsDigitChar(PeekChar(), 10)) {
              AddError(RangeFrom(backslash_start),
                       ErrorCode::STRING_LITERAL_BACKSLASH);
              state = State::Normal;
              continue;
            }
            characters.push_back('\0');
            state = State::Normal;
            continue;
          case 'b':
            state = State::Normal;
            characters.push_back('\b');
            break;
          case 'f':
            state = State::Normal;
            characters.push_back('\f');
            break;
          case 'n':
            state = State::Normal;
            characters.push_back('\n');
            break;
          case 'r':
            state = State::Normal;
            characters.push_back('\r');
            break;
          case 't':
            state = State::Normal;
            characters.push_back('\t');
            break;
          case 'v':
            state = State::Normal;
            characters.push_back('\v');
            break;
          case '\n':
          case 0x2028:  // Line separator
          case 0x2029:  // Paragraph separator
            state = State::Normal;
            break;
          case '\r':
            state = State::BackslashCr;
            break;
          case 'x':
            state = State::BackslashX;
            break;
          case 'u':
            state = State::BackslashU;
            break;
          default:
            characters.push_back(ConsumeChar());
            if (options_.enable_strict_backslash) {
              AddError(RangeFrom(backslash_start),
                       ErrorCode::STRING_LITERAL_BACKSLASH);
            }
            state = State::Normal;
            continue;
        }
        break;
      case State::BackslashCr:
        state = State::Normal;
        if (ConsumeCharIf('\n'))
          break;
        continue;
      case State::BackslashU:
        if (IsDigitChar(PeekChar(), 16)) {
          state = State::BackslashU1;
          accumulator = FromDigitChar(PeekChar(), 16);
          break;
        }
        if (PeekChar() == '{') {
          state = State::BackslashUB;
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashU1:
        if (!IsDigitChar(PeekChar(), 16))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromDigitChar(PeekChar(), 16);
        state = State::BackslashU2;
        break;
      case State::BackslashU2:
        if (!IsDigitChar(PeekChar(), 16))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromDigitChar(PeekChar(), 16);
        state = State::BackslashU3;
        break;
      case State::BackslashU3:
        if (!IsDigitChar(PeekChar(), 16))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromDigitChar(PeekChar(), 16);
        characters.push_back(static_cast<base::char16>(accumulator));
        state = State::Normal;
        break;
      case State::BackslashUB:
        if (IsDigitChar(PeekChar(), 16)) {
          accumulator = FromDigitChar(PeekChar(), 16);
          state = State::BackslashUBx;
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashUBx:
        if (PeekChar() == '}') {
          if (accumulator > kMaxUnicodeCodePoint) {
            AddError(RangeFrom(backslash_start),
                     ErrorCode::STRING_LITERAL_BACKSLASH_UNICODE);
          } else {
            characters.push_back(static_cast<base::char16>(accumulator));
          }
          state = State::Normal;
          break;
        }
        if (IsDigitChar(PeekChar(), 16)) {
          accumulator *= 16;
          accumulator |= FromDigitChar(PeekChar(), 16);
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashX:
        if (!IsDigitChar(PeekChar(), 16))
          goto invalid_hex_digit;
        accumulator = FromDigitChar(PeekChar(), 16);
        state = State::BackslashX1;
        break;
      case State::BackslashX1:
        if (!IsDigitChar(PeekChar(), 16))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromDigitChar(PeekChar(), 16);
        characters.push_back(static_cast<base::char16>(accumulator));
        state = State::Normal;
        break;
      invalid_hex_digit:
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      default:
        NOTREACHED() << "Invalid state " << static_cast<int>(state);
        break;
    }
    ConsumeChar();
  }
  return NewError(ErrorCode::STRING_LITERAL_NOT_CLOSED);
}

SourceCodeRange Lexer::MakeTokenRange() const {
  if (reader_->CanPeekChar())
    DCHECK_LT(token_start_, reader_->location());
  return source_code().Slice(token_start_, reader_->location());
}

ast::Token& Lexer::NewError(ErrorCode error_code) {
  AddError(error_code);
  return NewInvalid(error_code);
}

ast::Token& Lexer::NewInvalid(ErrorCode error_code) {
  return node_factory().NewInvalid(MakeTokenRange(),
                                   static_cast<int>(error_code));
}

ast::Token& Lexer::NewPunctuator(ast::PunctuatorKind kind) {
  return node_factory().NewPunctuator(MakeTokenRange(), kind);
}

ast::Token* Lexer::NextToken() {
  is_separated_by_newline_ = false;
  while (reader_->CanPeekChar()) {
    if (IsLineTerminator(PeekChar()))
      is_separated_by_newline_ = true;
    if (!IsWhitespace(PeekChar()))
      return &HandleCharacter();
    ConsumeChar();
  }
  return nullptr;
}

SourceCodeRange Lexer::RangeFrom(int start) const {
  return source_code().Slice(start, reader_->location());
}

}  // namespace internal
}  // namespace joana

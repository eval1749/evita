// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <cmath>
#include <limits>
#include <vector>

#include "joana/parser/lexer/lexer.h"

#include "joana/parser/lexer/character_reader.h"
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

const int kMaxUnicodeCodePoint = 0x10FFFF;
const base::char16 kLineSeparator = 0x2028;
const base::char16 kParagraphSeparator = 0x2029;

int FromDigitWithBase(base::char16 char_code, int base) {
  DCHECK(base == 2 || base == 8 || base == 10 || base == 16) << base;
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

int FromHexDigit(base::char16 char_code) {
  return FromDigitWithBase(char_code, 16);
}

bool IsDigitWithBase(base::char16 char_code, int base) {
  DCHECK(base == 2 || base == 8 || base == 10 || base == 16) << base;
  if (base == 16) {
    if (char_code >= '0' && char_code <= '9')
      return true;
    if (char_code >= 'A' && char_code <= 'F')
      return true;
    return char_code >= 'a' && char_code <= 'f';
  }
  return char_code >= '0' && char_code <= '0' + base - 1;
}

bool IsHexDigit(base::char16 char_code) {
  return IsDigitWithBase(char_code, 16);
}

bool IsIdentifierPart(base::char16 char_code) {
  if (char_code >= '0' && char_code <= '9')
    return true;
  if (char_code >= 'A' && char_code <= 'Z')
    return true;
  if (char_code >= 'a' && char_code <= 'z')
    return true;
  // TODO(eval1749): NYI: UnicodeIDContinue
  // TODO(eval1749): NYI: \ UnicodeEscapeSequence
  // TODO(eval1749): NYI: <ZWNJ> <ZWJ>
  return char_code == '$' || char_code == '_';
}

bool IsLineTerminator(base::char16 char_code) {
  return char_code == '\n' || char_code == '\r' ||
         char_code == kLineSeparator || char_code == kParagraphSeparator;
}

bool IsWhitespace(base::char16 char_code) {
  return IsLineTerminator(char_code) || char_code == ' ' || char_code == '\t' ||
         char_code == '\v' || char_code == '\f' || char_code == '\r';
}

}  // namespace

enum class Lexer::ErrorCode {
  None = ast::kLexerErrorCodeBase,
#define V(token, reason) token##_##reason,
  FOR_EACH_LEXER_ERROR_CODE(V)
#undef V
};

Lexer::Lexer(ast::EditContext* context, const SourceCodeRange& range)
    : context_(context),
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
  return context_->node_factory();
}

const SourceCode& Lexer::source_code() const {
  return reader_->source_code();
}

void Lexer::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  context_->error_sink().AddError(range, static_cast<int>(error_code));
}

void Lexer::AddError(ErrorCode error_code) {
  AddError(MakeTokenRange(), error_code);
}

void Lexer::Advance() {
  DCHECK(current_token_);
  current_token_ = NextToken();
}

ast::Token& Lexer::PeekToken() const {
  DCHECK(current_token_);
  return *current_token_;
}

ast::Token& Lexer::HandleBlockComment() {
  auto is_after_asterisk = false;
  while (reader_->HasCharacter()) {
    if (is_after_asterisk && reader_->AdvanceIf('/'))
      return node_factory().NewComment(MakeTokenRange());
    is_after_asterisk = reader_->Consume() == '*';
  }
  return NewInvalid(ErrorCode::BLOCK_COMMENT_NOT_CLOSED);
}

ast::Token& Lexer::HandleCharacter() {
  token_start_ = reader_->location();
  switch (reader_->Get()) {
    case ' ':
    case 0x0009:  // t TAB
    case 0x000A:  // n LF
    case 0x000B:  // v VT
    case 0x000C:  // f FF
    case 0x000D:  // r CR
    case '!':
      reader_->Advance();
      if (reader_->AdvanceIf('=')) {
        if (reader_->AdvanceIf('='))
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
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::LeftParenthesis);
    case ')':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::RightParenthesis);
    case '*':
      reader_->Advance();
      if (reader_->AdvanceIf('='))
        return NewPunctuator(ast::PunctuatorKind::TimesEqual);
      if (reader_->AdvanceIf('*')) {
        if (reader_->AdvanceIf('='))
          return NewPunctuator(ast::PunctuatorKind::TimesTimesEqual);
        return NewPunctuator(ast::PunctuatorKind::TimesTimes);
      }
      return NewPunctuator(ast::PunctuatorKind::Times);
    case '+':
      return HandleOperator(ast::PunctuatorKind::Plus,
                            ast::PunctuatorKind::PlusPlus,
                            ast::PunctuatorKind::PlusEqual);
    case ',':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::Comma);
    case '-':
      return HandleOperator(ast::PunctuatorKind::Minus,
                            ast::PunctuatorKind::MinusMinus,
                            ast::PunctuatorKind::MinusEqual);
    case '.':
      reader_->Advance();
      if (reader_->AdvanceIf('.')) {
        if (reader_->AdvanceIf('.'))
          return NewPunctuator(ast::PunctuatorKind::DotDotDot);
        return NewError(ErrorCode::PUNCTUATOR_DOT_DOT);
      }
      return NewPunctuator(ast::PunctuatorKind::Dot);
    case '/':
      reader_->Advance();
      if (reader_->AdvanceIf('='))
        return NewPunctuator(ast::PunctuatorKind::DivideEqual);
      if (reader_->AdvanceIf('*'))
        return HandleBlockComment();
      if (reader_->AdvanceIf('/'))
        return HandleLineComment();
      return NewPunctuator(ast::PunctuatorKind::Divide);
    case '0':
      reader_->Advance();
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
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::Colon);
    case ';':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::SemiColon);
    case '<':
      reader_->Advance();
      if (reader_->AdvanceIf('='))
        return NewPunctuator(ast::PunctuatorKind::LessThanOrEqual);
      if (reader_->AdvanceIf('<')) {
        if (reader_->AdvanceIf('='))
          return NewPunctuator(ast::PunctuatorKind::LeftShiftEqual);
        return NewPunctuator(ast::PunctuatorKind::LeftShift);
      }
      return NewPunctuator(ast::PunctuatorKind::LessThan);
    case '=':
      if (reader_->AdvanceIf('=')) {
        if (reader_->AdvanceIf('='))
          return NewPunctuator(ast::PunctuatorKind::EqualEqualEqual);
        return NewPunctuator(ast::PunctuatorKind::EqualEqual);
      }
      if (reader_->AdvanceIf('>'))
        return NewPunctuator(ast::PunctuatorKind::Arrow);
      return NewPunctuator(ast::PunctuatorKind::Equal);
    case '>':
      reader_->Advance();
      if (reader_->AdvanceIf('='))
        return NewPunctuator(ast::PunctuatorKind::GreaterThanOrEqual);
      if (reader_->AdvanceIf('>')) {
        if (reader_->AdvanceIf('='))
          return NewPunctuator(ast::PunctuatorKind::RightShiftEqual);
        if (reader_->AdvanceIf('>')) {
          if (reader_->AdvanceIf('='))
            return NewPunctuator(ast::PunctuatorKind::UnsignedRightShiftEqual);
          return NewPunctuator(ast::PunctuatorKind::UnsignedRightShift);
        }
        return NewPunctuator(ast::PunctuatorKind::RightShift);
      }
      return NewPunctuator(ast::PunctuatorKind::GreaterThan);
    case '?':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::Question);
    case '`':
      // TODO(eval1749): NYI: template token
      return HandleStringLiteral();
    case '[':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::LeftBracket);
    case ']':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::RightBracket);
    case '_':
      return HandleName();
    case '^':
      return HandleOperator(ast::PunctuatorKind::BitXor,
                            ast::PunctuatorKind::Invalid,
                            ast::PunctuatorKind::BitXorEqual);
    case '{':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::LeftBrace);
    case '}':
      reader_->Advance();
      return NewPunctuator(ast::PunctuatorKind::RightBrace);
    case '|':
      return HandleOperator(ast::PunctuatorKind::BitOr,
                            ast::PunctuatorKind::LogicalOr,
                            ast::PunctuatorKind::BitOrEqual);
    invalid_character:
    default:
      if (reader_->Get() >= 'A' && reader_->Get() <= 'Z')
        return HandleName();
      if (reader_->Get() >= 'a' && reader_->Get() <= 'z')
        return HandleName();
      // TODO(eval1749): NYI: UnicodeIDStart
      // TODO(eval1749): NYI: \ UnicodeEscapeSequence
      reader_->Advance();
      return NewInvalid(ErrorCode::CHARACTER_INVALID);
  }
}

ast::Token& Lexer::HandleDecimal() {
  const auto kMaxIntegerPart = std::numeric_limits<uint64_t>::max() / 10 - 1;
  uint64_t digits_part = reader_->Consume() - '0';
  auto digits_scale = 0;
  while (reader_->HasCharacter()) {
    if (!IsDigitWithBase(reader_->Get(), 10))
      break;
    const auto digit = FromDigitWithBase(reader_->Consume(), 10);
    if (digits_part > kMaxIntegerPart) {
      ++digits_scale;
      continue;
    }
    digits_part *= 10;
    digits_part += digit;
  }
  if (reader_->AdvanceIf('.')) {
    while (reader_->HasCharacter()) {
      if (!IsDigitWithBase(reader_->Get(), 10))
        break;
      const auto digit = FromDigitWithBase(reader_->Consume(), 10);
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
  if (reader_->AdvanceIf('E') || reader_->AdvanceIf('e')) {
    if (reader_->AdvanceIf('+'))
      exponent_sign = 1;
    else if (reader_->AdvanceIf('-'))
      exponent_sign = -1;
    while (reader_->HasCharacter()) {
      if (!IsDigitWithBase(reader_->Get(), 10))
        break;
      const auto digit = FromDigitWithBase(reader_->Consume(), 10);
      if (exponent_part > kMaxIntegerPart)
        continue;
      exponent_part *= 10;
      exponent_part += digit;
    }
  }
  const auto value =
      static_cast<double>(digits_part) * std::pow(10.0, digits_scale);
  const auto exponent = exponent_part * exponent_sign;
  return node_factory().NewNumericLiteral(MakeTokenRange(),
                                          value * std::pow(10.0, exponent));
}

ast::Token& Lexer::HandleDigitZero() {
  switch (reader_->Get()) {
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
      reader_->Advance();
      return HandleInteger(2);
    case 'o':
    case 'O':
      reader_->Advance();
      return HandleInteger(8);
    case 'x':
    case 'X':
      reader_->Advance();
      return HandleInteger(16);
  }
  return NewInvalid(ErrorCode::NUMERIC_LITERAL_PREFIX_ZERO);
}

ast::Token& Lexer::HandleInteger(int base) {
  DCHECK(base == 2 || base == 8 || base == 16) << base;
  uint64_t accumulator = 0;
  const auto kMaxInteger = static_cast<uint64_t>(1) << 53;
  auto number_of_digits = 0;
  auto is_overflow = false;
  while (reader_->HasCharacter() && IsDigitWithBase(reader_->Get(), base)) {
    const auto digit = FromDigitWithBase(reader_->Consume(), base);
    if (accumulator > kMaxInteger / base) {
      if (!is_overflow)
        AddError(ErrorCode::NUMERIC_LITERAL_INTEGER_OVERFLOW);
      is_overflow = true;
    } else {
      accumulator *= base;
      accumulator += digit;
    }
    ++number_of_digits;
  }

  if (reader_->HasCharacter() && IsIdentifierPart(reader_->Get())) {
    reader_->Advance();
    AddError(ErrorCode::NUMERIC_LITERAL_INTEGER_BAD_DIGIT);
    while (reader_->HasCharacter() && IsIdentifierPart(reader_->Get()))
      reader_->Advance();
    return NewInvalid(ErrorCode::NUMERIC_LITERAL_INTEGER_BAD_DIGIT);
  }
  if (is_overflow)
    return NewInvalid(ErrorCode::NUMERIC_LITERAL_INTEGER_OVERFLOW);
  if (number_of_digits == 0)
    return NewError(ErrorCode::NUMERIC_LITERAL_INTEGER_NO_DIGITS);
  return node_factory().NewNumericLiteral(MakeTokenRange(), accumulator);
}

ast::Token& Lexer::HandleLineComment() {
  while (reader_->HasCharacter()) {
    if (IsLineTerminator(reader_->Consume()))
      break;
  }
  return node_factory().NewComment(MakeTokenRange());
}

ast::Token& Lexer::HandleName() {
  while (reader_->HasCharacter()) {
    if (!IsIdentifierPart(reader_->Get()))
      break;
    reader_->Advance();
  }
  return node_factory().NewName(MakeTokenRange());
}

// Handle op, op op, op '=' pattern.
ast::Token& Lexer::HandleOperator(ast::PunctuatorKind one,
                                  ast::PunctuatorKind two,
                                  ast::PunctuatorKind equal) {
  const auto char_code = reader_->Consume();
  if (reader_->AdvanceIf('='))
    return NewPunctuator(equal);
  if (two != ast::PunctuatorKind::Invalid && reader_->AdvanceIf(char_code))
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
  const auto delimiter = reader_->Consume();
  auto accumulator = 0;
  auto backslash_start = 0;
  while (reader_->HasCharacter()) {
    switch (state) {
      case State::Normal:
        if (reader_->AdvanceIf(delimiter)) {
          return node_factory().NewStringLiteral(
              MakeTokenRange(),
              base::StringPiece16(characters.data(), characters.size()));
        }
        if (reader_->Get() == '\\') {
          backslash_start = reader_->location();
          state = State::Backslash;
          break;
        }
        if (IsLineTerminator(reader_->Get())) {
          AddError(RangeFrom(token_start_), ErrorCode::STRING_LITERAL_NEWLINE);
        }
        characters.push_back(reader_->Get());
        break;
      case State::Backslash:
        switch (reader_->Get()) {
          case '\'':
          case '"':
          case '\\':
            state = State::Normal;
            characters.push_back(reader_->Get());
            break;
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
            AddError(RangeFrom(backslash_start),
                     ErrorCode::STRING_LITERAL_BACKSLASH);
            state = State::Normal;
            break;
        }
        break;
      case State::BackslashCr:
        state = State::Normal;
        if (reader_->AdvanceIf('\n'))
          break;
        continue;
      case State::BackslashU:
        if (IsHexDigit(reader_->Get())) {
          state = State::BackslashU1;
          accumulator = FromHexDigit(reader_->Get());
          break;
        }
        if (reader_->Get() == '{') {
          state = State::BackslashUB;
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashU1:
        if (!IsHexDigit(reader_->Get()))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromHexDigit(reader_->Get());
        state = State::BackslashU2;
        break;
      case State::BackslashU2:
        if (!IsHexDigit(reader_->Get()))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromHexDigit(reader_->Get());
        state = State::BackslashU3;
        break;
      case State::BackslashU3:
        if (!IsHexDigit(reader_->Get()))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromHexDigit(reader_->Get());
        characters.push_back(static_cast<base::char16>(accumulator));
        state = State::Normal;
        break;
      case State::BackslashUB:
        if (IsHexDigit(reader_->Get())) {
          accumulator = FromHexDigit(reader_->Get());
          state = State::BackslashUBx;
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashUBx:
        if (reader_->Get() == '}') {
          if (accumulator > kMaxUnicodeCodePoint) {
            AddError(RangeFrom(backslash_start),
                     ErrorCode::STRING_LITERAL_BACKSLASH_UNICODE);
          } else {
            characters.push_back(static_cast<base::char16>(accumulator));
          }
          state = State::Normal;
          break;
        }
        if (IsHexDigit(reader_->Get())) {
          accumulator *= 16;
          accumulator |= FromHexDigit(reader_->Get());
          break;
        }
        AddError(RangeFrom(backslash_start),
                 ErrorCode::STRING_LITERAL_BACKSLASH_HEX_DIGIT);
        state = State::Normal;
        break;
      case State::BackslashX:
        if (!IsHexDigit(reader_->Get()))
          goto invalid_hex_digit;
        accumulator = FromHexDigit(reader_->Get());
        state = State::BackslashX1;
        break;
      case State::BackslashX1:
        if (!IsHexDigit(reader_->Get()))
          goto invalid_hex_digit;
        accumulator *= 16;
        accumulator |= FromHexDigit(reader_->Get());
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
    reader_->Advance();
  }
  return NewError(ErrorCode::STRING_LITERAL_NOT_CLOSED);
}

SourceCodeRange Lexer::MakeTokenRange() const {
  if (reader_->HasCharacter())
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
  while (reader_->HasCharacter()) {
    if (!IsWhitespace(reader_->Get()))
      return &HandleCharacter();
    reader_->Advance();
  }
  return nullptr;
}

SourceCodeRange Lexer::RangeFrom(int start) const {
  return source_code().Slice(start, reader_->location());
}

}  // namespace internal
}  // namespace joana

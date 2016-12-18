// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_LEXER_H_
#define JOANA_PARSER_LEXER_LEXER_H_

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "joana/public/ast/node_forward.h"

namespace joana {

class ErrorSink;
class PunctuatorKind;
class SourceCode;
class SourceCodeRange;

namespace internal {

class CharacterReader;

class Lexer final {
 public:
  // Expose |ErrorCode| for RegExp parser.
  enum class ErrorCode;

  Lexer(ast::EditContext* context, const SourceCodeRange& range);

  ~Lexer();

  // Returns source code location where |Lexer| reads.
  SourceCodeRange location() const;
  const SourceCode& source_code() const;

  void Advance();

  // Returns true if |Lexer| has a token.
  bool CanPeekToken() const { return current_token_ != nullptr; }

  // Returns |ast::RegExp| after "/".
  ast::RegExp& ConsumeRegExp();

  ast::Token& PeekToken() const;

 private:
  ast::NodeFactory& node_factory() const;

  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);

  // Returns true if |Lexer| has a character.
  bool CanPeekChar() const;

  base::char16 ConsumeChar();

  // Returns true if |PeekChar()| is |expected_char| and advance to next
  // character.
  bool ConsumeCharIf(base::char16 expected_char);

  // Returns character.
  base::char16 PeekChar() const;
  ast::Token& HandleBlockComment();
  ast::Token& HandleCharacter();
  ast::Token& HandleDecimal();
  ast::Token& HandleDecimalAfterDot(uint64_t integer_part, int integer_scale);
  ast::Token& HandleDigitZero();
  ast::Token& HandleInteger(int base);
  ast::Token& HandleLineComment();
  ast::Token& HandleName();
  ast::Token& HandleOperator(ast::PunctuatorKind one,
                             ast::PunctuatorKind two,
                             ast::PunctuatorKind equal);
  ast::Token& HandleStringLiteral();

  SourceCodeRange MakeTokenRange() const;

  ast::Token& NewError(ErrorCode error_code);
  ast::Token& NewInvalid(ErrorCode error_code);
  ast::Token& NewPunctuator(ast::PunctuatorKind kind);

  ast::Token* NextToken();

  SourceCodeRange RangeFrom(int start) const;

  ast::EditContext& context_;
  ast::Token* current_token_ = nullptr;
  const SourceCodeRange& range_;
  const std::unique_ptr<CharacterReader> reader_;
  int token_start_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_LEXER_H_

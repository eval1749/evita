// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_LEXER_H_
#define JOANA_PARSER_LEXER_LEXER_H_

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "joana/ast/syntax_forward.h"

namespace joana {

class ErrorSink;
class ParserContext;
class ParserOptions;
struct PunctuatorKind;
class SourceCode;
class SourceCodeRange;

namespace ast {
class Node;
class NodeFactory;
}

namespace parser {

class CharacterReader;

//
// Lexer
//
class Lexer final {
 public:
  // Expose |ErrorCode| for RegExp parser.
  enum class ErrorCode;

  Lexer(ParserContext* context,
        const SourceCodeRange& range,
        const ParserOptions& options);

  ~Lexer();

  // Returns source code location where |Lexer| reads.
  SourceCodeRange location() const;

  // Returns true if there are at least one line terminate between current token
  // and previous token.
  bool is_separated_by_newline() const { return is_separated_by_newline_; }

  const SourceCode& source_code() const;

  // Returns true if |Lexer| has a token.
  bool CanPeekToken() const { return current_token_ != nullptr; }

  // Returns |ast::RegExp| after "/".
  const ast::Node& ConsumeRegExp();

  const ast::Node& ConsumeToken();

  // Returns |RegExpSource| node starts with "/" or "/=", and ends with "/".
  const ast::Node& ExtendTokenAsRegExp();

  const ast::Node& PeekToken() const;

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
  const ast::Node& HandleBlockComment();
  const ast::Node* HandleCharacter();
  const ast::Node& HandleDecimal();
  const ast::Node& HandleDecimalAfterDot(uint64_t integer_part,
                                         int integer_scale);
  const ast::Node& HandleDigitZero();
  const ast::Node& HandleInteger(int base);
  const ast::Node& HandleLineComment();
  const ast::Node& HandleName();
  const ast::Node& HandleOperator(ast::PunctuatorKind one,
                                  ast::PunctuatorKind two,
                                  ast::PunctuatorKind equal);
  const ast::Node& HandleStringLiteral();

  SourceCodeRange MakeTokenRange() const;

  const ast::Node& NewNumericLiteral(double value);
  const ast::Node& NewPunctuator(ast::PunctuatorKind kind);

  const ast::Node* NextToken();

  SourceCodeRange RangeFrom(int start) const;

  ParserContext& context_;
  const ast::Node* current_token_ = nullptr;

  // True if current token and previous token is separated by at least one
  // line terminator.
  bool is_separated_by_newline_ = false;
  const ParserOptions& options_;
  const SourceCodeRange& range_;
  const std::unique_ptr<CharacterReader> reader_;
  int token_start_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_LEXER_H_

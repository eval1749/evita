// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_LEXER_H_
#define JOANA_PARSER_LEXER_LEXER_H_

#include <memory>

#include "base/macros.h"
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
  Lexer(ast::EditContext* context, const SourceCodeRange& range);

  ~Lexer();

  // Returns source code location where |Lexer| reads.
  SourceCodeRange location() const;
  const SourceCode& source_code() const;

  void Advance();
  // Returns true if |Lexer| has a token.
  bool HasToken() const { return current_token_ != nullptr; }
  ast::Node& GetToken() const;

 private:
  enum class ErrorCode;

  ast::NodeFactory& node_factory() const;

  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);

  ast::Node& HandleBlockComment();
  ast::Node& HandleCharacter();
  ast::Node& HandleDecimal();
  ast::Node& HandleDigitZero();
  ast::Node& HandleInteger(int base);
  ast::Node& HandleLineComment();
  ast::Node& HandleName();
  ast::Node& HandleOperator(ast::PunctuatorKind one,
                            ast::PunctuatorKind two,
                            ast::PunctuatorKind equal);
  ast::Node& HandleStringLiteral();

  SourceCodeRange MakeTokenRange() const;

  ast::Node& NewError(ErrorCode error_code);
  ast::Node& NewInvalid(ErrorCode error_code);
  ast::Node& NewPunctuator(ast::PunctuatorKind kind);

  ast::Node* NextToken();

  SourceCodeRange RangeFrom(int start) const;

  ast::EditContext* const context_;
  ast::Node* current_token_ = nullptr;
  const SourceCodeRange& range_;
  const std::unique_ptr<CharacterReader> reader_;
  int token_start_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_LEXER_H_

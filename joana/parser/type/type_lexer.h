// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_TYPE_TYPE_LEXER_H_
#define JOANA_PARSER_TYPE_TYPE_LEXER_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "joana/parser/public/parser_options.h"

namespace joana {
class ParserContext;
class SourceCode;
class SourceCodeRange;

namespace ast {
class Node;
class NodeFactory;
enum class PunctuatorKind;
using Token = Node;
}
namespace parser {

class CharacterReader;
enum class TypeErrorCode;

enum class TypeLexerMode {
  Normal,
  JsDoc,
};

//
// TypeLexer
//
class TypeLexer final {
 public:
  TypeLexer(ParserContext* context,
            const SourceCodeRange& range,
            const ParserOptions& options,
            TypeLexerMode mode);
  ~TypeLexer();

  SourceCodeRange location() const;
  const SourceCodeRange& range() const;
  const SourceCode& source_code() const;

  bool CanPeekToken() const { return current_token_ != nullptr; }
  const ast::Token& ConsumeToken();
  const ast::Token& PeekToken() const;

 private:
  ast::NodeFactory& node_factory();

  void AddError(TypeErrorCode error_code);
  bool CanPeekChar() const;
  base::char16 ConsumeChar();
  bool ConsumeCharIf(base::char16 char_code);
  base::char16 PeekChar() const;

  const ast::Token* NextToken();

  // Factory members
  SourceCodeRange ComputeTokenRange() const;
  const ast::Token& NewPunctuator(ast::PunctuatorKind kind);

  ParserContext& context_;
  const ast::Token* current_token_ = nullptr;
  const TypeLexerMode mode_;
  ParserOptions options_;
  const std::unique_ptr<CharacterReader> reader_;
  int token_start_ = 0;

  DISALLOW_COPY_AND_ASSIGN(TypeLexer);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_TYPE_TYPE_LEXER_H_

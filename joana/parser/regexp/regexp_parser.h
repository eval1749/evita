// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_REGEXP_REGEXP_PARSER_H_
#define JOANA_PARSER_REGEXP_REGEXP_PARSER_H_

#include <memory>
#include <stack>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
class NodeFactory;
enum class PunctuatorKind;
}

class ParserContext;
class ParserOptions;
class SourceCode;
class SourceCodeRange;

namespace parser {

class RegExpLexer;

//
// RegExpParser
//
class RegExpParser final {
 public:
  RegExpParser(ParserContext* context,
               const SourceCodeRange& range,
               const ParserOptions& options);
  ~RegExpParser();

  const ast::Node& Parse();

 private:
  friend class ScopedNodeFactory;

  int location() const;
  ast::NodeFactory& node_factory() const;
  const SourceCode& source_code() const;

  const ast::Node& ParseOr();
  const ast::Node& ParsePrimary();
  const ast::Node& ParseParenthesis();
  const ast::Node& ParseRepeat();
  const ast::Node& ParseSequence();

  // Helper functions for Lexer
  bool CanPeekToken() const;
  void ConsumeToken();
  bool ConsumeTokenIf(ast::PunctuatorKind kind);
  const ast::Node& PeekToken() const;

  ParserContext& context_;

  // |groups_| holds start offset of left parenthesis for checking matched
  // parenthesis.
  std::stack<int> groups_;

  const ParserOptions& options_;

  // The last consumed token before |ConsumeTokenIf()|.
  const ast::Node* last_token_ = nullptr;

  const std::unique_ptr<RegExpLexer> lexer_;

  DISALLOW_COPY_AND_ASSIGN(RegExpParser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_REGEXP_REGEXP_PARSER_H_

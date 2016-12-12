// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>

#include "base/macros.h"

namespace joana {
namespace ast {
class ContainerNode;
class EditContext;
class Expression;
class Literal;
class Name;
enum class NameId;
class Node;
class NodeFactory;
enum class PunctuatorKind;
class Statement;
}  // namespace ast
class SourceCode;
class SourceCodeRange;

namespace internal {

class Lexer;

class Parser final {
 public:
  Parser(ast::EditContext* context, const SourceCodeRange& range);

  ~Parser();

  // Returns root node, either |Module| or |Script|, of abstract syntax tree
  // for source code specified in |range|. The result may be partial result
  // if source code does not match grammar.
  const ast::Node& Run();

 private:
  class BracketStack;
  enum class ErrorCode;
  class StatementScope;

  ast::NodeFactory& node_factory() const;
  const SourceCode& source_code() const;

  void AddError(const ast::Node& token, ErrorCode error_code);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);

  void Advance();
  ast::Node& ComputeInvalidToken(ErrorCode error_code);
  ast::Node& ConsumeToken();
  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);
  void ExpectToken(ast::NameId name_id, ErrorCode error_code);
  void ExpectToken(ast::PunctuatorKind kind, ErrorCode error_code);
  ast::Node& GetLastToken();
  ast::Node& PeekToken();
  bool HasToken() const;

  // Expressions
  ast::Expression& NewInvalidExpression(ErrorCode error_code);
  ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  ast::Expression& ParseExpression();
  ast::Expression& ParseExpressionAfterName(const ast::Name& name);

  // Statements
  bool CanUseBreak() const;
  bool CanUseContinue() const;
  ast::Statement& NewInvalidStatement(ErrorCode error_code);
  ast::Statement& ParseStatement();

  ast::Statement& ParseAsyncStatement();
  ast::Statement& ParseBreakStatement();
  ast::Statement& ParseBlockStatement();
  ast::Statement& ParseCaseClause();
  ast::Statement& ParseConstStatement();
  ast::Statement& ParseContinueStatement();
  ast::Statement& ParseDefaultLabel();
  ast::Statement& ParseDoStatement();
  ast::Statement& ParseExpressionStatement();
  ast::Statement& ParseForStatement();
  ast::Statement& ParseFunctionStatement();
  ast::Statement& ParseIfStatement();
  ast::Statement& ParseKeywordStatement();
  ast::Statement& ParseLetStatement();
  ast::Statement& ParseReturnStatement();
  ast::Statement& ParseSwitchStatement();
  ast::Statement& ParseThrowStatement();
  ast::Statement& ParseTryStatement();
  ast::Statement& ParseVarStatement();
  ast::Statement& ParseWhileStatement();
  ast::Statement& ParseYieldStatement();

  std::unique_ptr<BracketStack> bracket_stack_;
  ast::EditContext* const context_;
  std::unique_ptr<Lexer> lexer_;
  ast::ContainerNode& root_;
  StatementScope* statement_scope_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_

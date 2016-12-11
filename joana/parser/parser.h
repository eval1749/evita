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

  void Advance();
  ast::Node& ComputeInvalidToken(ErrorCode error_code);
  ast::Node& ConsumeToken();
  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);
  void ExpectToken(ast::PunctuatorKind kind, ErrorCode error_code);
  ast::Node& GetLastToken();
  ast::Node& PeekToken();
  bool HasToken() const;

  // Expressions
  ast::Expression& NewInvalidExpression(ErrorCode error_code);
  ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  ast::Expression& ParseExpression();
  ast::Expression& ParseExpressionName();

  // Statements
  bool CanUseBreak() const;
  ast::Statement& NewInvalidStatement(ErrorCode error_code);
  ast::Statement& ParseStatement();
  ast::Statement& ParseStatementAsync();
  ast::Statement& ParseStatementBreak();
  ast::Statement& ParseStatementBlock();
  ast::Statement& ParseStatementConst();
  ast::Statement& ParseStatementContinue();
  ast::Statement& ParseStatementDo();
  ast::Statement& ParseStatementExpression();
  ast::Statement& ParseStatementFor();
  ast::Statement& ParseStatementFunction();
  ast::Statement& ParseStatementIf();
  ast::Statement& ParseStatementKeyword();
  ast::Statement& ParseStatementLet();
  ast::Statement& ParseStatementReturn();
  ast::Statement& ParseStatementSwitch();
  ast::Statement& ParseStatementThrow();
  ast::Statement& ParseStatementVar();
  ast::Statement& ParseStatementWhile();
  ast::Statement& ParseStatementYield();

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

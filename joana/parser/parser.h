// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>
#include <stack>
#include <vector>

#include "base/macros.h"
#include "joana/public/ast/node_forward.h"

namespace joana {

class SourceCode;
class SourceCodeRange;

namespace internal {

class Lexer;

class Parser final {
 public:
  // Expose |OperatorPrecedence| for implementing static helper functions.
  enum class OperatorPrecedence;

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

  class SourceCodeRangeScope final {
   public:
    explicit SourceCodeRangeScope(Parser* parser);
    ~SourceCodeRangeScope();

   private:
    Parser* const parser_;

    DISALLOW_COPY_AND_ASSIGN(SourceCodeRangeScope);
  };

  ast::NodeFactory& node_factory() const;
  const SourceCode& source_code() const;

  void AddError(const ast::Node& token, ErrorCode error_code);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);

  void Advance();
  ast::Token& ComputeInvalidToken(ErrorCode error_code);
  ast::Token& ConsumeToken();
  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);
  void ExpectToken(ast::NameId name_id, ErrorCode error_code);
  void ExpectToken(ast::PunctuatorKind kind, ErrorCode error_code);
  SourceCodeRange GetSourceCodeRange() const;
  bool HasToken() const;
  ast::Token& PeekToken();
  void PushBackToken(const ast::Token& token);

  // Declarations
  ast::Token& NewEmptyName();
  ast::ArrowFunctionBody& ParseArrowFunctionBody();
  ast::Function& ParseFunction(ast::FunctionKind kind);
  ast::Statement& ParseFunctionBody();
  ast::Expression& ParseParameterList();

  // Expressions
  OperatorPrecedence CategoryOf(const ast::Token& token) const;
  OperatorPrecedence HigherPrecedenceOf(OperatorPrecedence category) const;
  ast::Expression& NewDeclarationExpression(
      const ast::Declaration& declaration);
  ast::Expression& NewElisionExpression();
  ast::Expression& NewInvalidExpression(ErrorCode error_code);
  ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  ast::Expression& NewUnaryExpression(const ast::Token& op,
                                      const ast::Expression& expression);

  // The entry of parsing an expression.
  ast::Expression& ParseExpression();

  std::vector<ast::Expression*> ParseArgumentList();
  ast::Expression& ParseArrayLiteralExpression();
  ast::Expression& ParseAssignmentExpression();
  ast::Expression& ParseBinaryExpression(OperatorPrecedence category);
  ast::Expression& ParseCommaExpression();
  ast::Expression& ParseConditionalExpression();
  ast::Expression& ParseFunctionExpression(ast::FunctionKind kind);
  ast::Expression& ParseLeftHandSideExpression();
  ast::Expression& ParseNameAsExpression(const ast::Name& name);
  ast::Expression& ParseNewExpression();
  ast::Expression& ParseParenthesis();
  ast::Expression& ParsePrimaryExpression();
  ast::Expression& ParseUnaryExpression();
  ast::Expression& ParseUpdateExpression();
  ast::Expression& ParseYieldExpression();

  // Statements
  bool CanUseBreak() const;
  bool CanUseContinue() const;
  ast::Statement& NewInvalidStatement(ErrorCode error_code);
  ast::Statement& ParseStatement();

  ast::Statement& ParseBreakStatement();
  ast::Statement& ParseBlockStatement();
  ast::Statement& ParseCaseClause();
  ast::Statement& ParseConstStatement();
  ast::Statement& ParseContinueStatement();
  ast::Statement& ParseDefaultLabel();
  ast::Statement& ParseDoStatement();
  ast::Statement& ParseExpressionStatement();
  ast::Statement& ParseForStatement();
  ast::Statement& ParseFunctionStatement(ast::FunctionKind kind);
  ast::Statement& ParseIfStatement();
  ast::Statement& ParseKeywordStatement();
  ast::Statement& ParseLetStatement();
  ast::Statement& ParseReturnStatement();
  ast::Statement& ParseSwitchStatement();
  ast::Statement& ParseThrowStatement();
  ast::Statement& ParseTryStatement();
  ast::Statement& ParseVarStatement();
  ast::Statement& ParseWhileStatement();

  std::unique_ptr<BracketStack> bracket_stack_;
  ast::EditContext* const context_;
  std::unique_ptr<Lexer> lexer_;
  ast::ContainerNode& root_;
  std::stack<int> range_stack_;
  StatementScope* statement_scope_ = nullptr;
  std::vector<ast::Token*> tokens_;
  std::stack<ast::Token*> token_stack_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_

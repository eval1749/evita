// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>
#include <stack>
#include <vector>

#include "base/macros.h"
#include "joana/ast/node_forward.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parse.h"

namespace joana {

class SourceCode;
class SourceCodeRange;

namespace internal {

class Lexer;

//
// Parser
//
class Parser final {
 public:
  // Expose |OperatorPrecedence| for implementing static helper functions.
  enum class OperatorPrecedence;

  Parser(ast::EditContext* context,
         const SourceCodeRange& range,
         const ParserOptions& options);

  ~Parser();

  // Returns root node, either |Module| or |Script|, of abstract syntax tree
  // for source code specified in |range|. The result may be partial result
  // if source code does not match grammar.
  const ast::Node& Run();

 private:
  class BracketStack;
  enum class ErrorCode;
  class ExpectSemicolonScope;

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
  bool CanPeekToken() const;
  ast::Token& ComputeInvalidToken(ErrorCode error_code);
  ast::Token& ConsumeToken();
  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);
  void ExpectPunctuator(ast::PunctuatorKind kind, ErrorCode error_code);
  void ExpectSemicolon();
  SourceCodeRange GetSourceCodeRange() const;
  ast::Token& PeekToken() const;
  void PushBackToken(const ast::Token& token);
  void SkipCommentTokens();

  // Declarations
  const ast::Token& NewEmptyName();
  const ast::ArrowFunctionBody& ParseArrowFunctionBody();
  const ast::Class& ParseClass();
  const ast::Expression& ParseClassBody();
  const ast::Expression& ParseClassHeritage();
  const ast::Token& ParseClassName();
  const ast::Function& ParseFunction(ast::FunctionKind kind);
  const ast::Statement& ParseFunctionBody();
  const ast::Method& ParseMethod(ast::MethodKind is_static,
                                 ast::FunctionKind kind);
  const ast::Expression& ParseParameterList();
  const ast::Expression& ParsePropertyName();

  // Expressions
  OperatorPrecedence CategoryOf(const ast::Token& token) const;

  const ast::Expression& HandleComputedMember(
      const ast::Expression& expression);
  const ast::Expression& HandleMember(const ast::Expression& expression);
  const ast::Expression& HandleNewExpression(const ast::Expression& expression);

  OperatorPrecedence HigherPrecedenceOf(OperatorPrecedence category) const;
  const ast::Expression& NewDeclarationExpression(
      const ast::Declaration& declaration);
  const ast::Expression& NewElisionExpression();
  const ast::Expression& NewEmptyExpression();
  const ast::Expression& NewInvalidExpression(const ast::Token& token,
                                              ErrorCode error_code);
  const ast::Expression& NewInvalidExpression(ErrorCode error_code);
  const ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  const ast::Expression& NewUnaryExpression(const ast::Token& op,
                                            const ast::Expression& expression);

  // The entry of parsing an expression.
  const ast::Expression& ParseExpression();

  // Helper function for parsing an expression enclosed by parenthesis for
  // do-while, if, switch, and while statements.
  const ast::Expression& ParseParenthesisExpression();

  const ast::RegExp& ParseRegExp();

  std::vector<const ast::Expression*> ParseArgumentList();
  const ast::Expression& ParseArrayLiteralExpression();
  const ast::Expression& ParseAssignmentExpression();
  const ast::Expression& ParseBinaryExpression(OperatorPrecedence category);
  const ast::Expression& ParseCommaExpression();
  const ast::Expression& ParseConditionalExpression();
  const ast::Expression& ParseFunctionExpression(ast::FunctionKind kind);
  const ast::Expression& ParseLeftHandSideExpression();
  const ast::Expression& ParseMethodExpression(ast::MethodKind is_static,
                                               ast::FunctionKind kind);
  const ast::Expression& ParseNameAsExpression();
  const ast::Expression& ParseNewExpression();
  const ast::Expression& ParseObjectLiteralExpression();
  const ast::Expression& ParseParenthesis();
  const ast::Expression& ParsePrimaryExpression();
  const ast::Expression& ParsePropertyAfterName(
      const ast::Expression& property_name,
      ast::MethodKind is_static,
      ast::FunctionKind function_kind);
  const ast::Expression& ParseRegExpLiteral();
  const ast::Expression& ParseUnaryExpression();
  const ast::Expression& ParseUpdateExpression();
  const ast::Expression& ParseYieldExpression();

  // Statements
  const ast::Statement& HandleLabeledStatement(const ast::Name& name);

  const ast::Statement& NewInvalidStatement(ErrorCode error_code);
  const ast::Statement& NewEmptyStatement(const SourceCodeRange& range);

  const ast::Statement& ParseStatement();

  const ast::Statement& ParseBreakStatement();
  const ast::Statement& ParseBlockStatement();
  const ast::Statement& ParseCaseClause();
  const ast::Statement& ParseClassStatement();
  const ast::Statement& ParseConstStatement();
  const ast::Statement& ParseContinueStatement();
  const ast::Statement& ParseDefaultLabel();
  const ast::Statement& ParseDoStatement();
  const ast::Statement& ParseExpressionStatement();
  const ast::Statement& ParseForStatement();
  const ast::Statement& ParseFunctionStatement(ast::FunctionKind kind);
  const ast::Statement& ParseIfStatement();
  const ast::Statement& ParseKeywordStatement();
  const ast::Statement& ParseLetStatement();
  const ast::Statement& ParseNameAsStatement();
  const ast::Statement& ParseReturnStatement();
  const ast::Statement& ParseSwitchStatement();
  const ast::Statement& ParseThrowStatement();
  const ast::Statement& ParseTryStatement();
  const ast::Statement& ParseVarStatement();
  const ast::Statement& ParseWhileStatement();
  const ast::Statement& ParseWithStatement();

  std::unique_ptr<BracketStack> bracket_stack_;
  ast::EditContext& context_;
  std::unique_ptr<Lexer> lexer_;

  // True if current token and previous token is separated by at least one
  // line terminator.
  bool is_separated_by_newline_ = false;

  ast::ContainerNode& root_;
  const ParserOptions& options_;
  std::stack<int> range_stack_;

  // List of tokens to locate comment.
  std::vector<ast::Token*> tokens_;

  // |token_stack_| is used for look ahead, e.g. detecting whether name is
  // part of an expression or label.
  std::stack<ast::Token*> token_stack_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_

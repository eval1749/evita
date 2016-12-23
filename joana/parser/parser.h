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
  ast::Token& NewEmptyName();
  ast::ArrowFunctionBody& ParseArrowFunctionBody();
  ast::Class& ParseClass();
  ast::Expression& ParseClassBody();
  ast::Expression& ParseClassHeritage();
  ast::Token& ParseClassName();
  ast::Function& ParseFunction(ast::FunctionKind kind);
  ast::Statement& ParseFunctionBody();
  ast::Method& ParseMethod(ast::MethodKind is_static, ast::FunctionKind kind);
  ast::Expression& ParseParameterList();
  ast::Expression& ParsePropertyName();

  // Expressions
  OperatorPrecedence CategoryOf(const ast::Token& token) const;

  ast::Expression& HandleComputedMember(ast::Expression* expression);
  ast::Expression& HandleMember(ast::Expression* expression);
  ast::Expression& HandleNewExpression(ast::Expression* expression);

  OperatorPrecedence HigherPrecedenceOf(OperatorPrecedence category) const;
  ast::Expression& NewDeclarationExpression(
      const ast::Declaration& declaration);
  ast::Expression& NewElisionExpression();
  ast::Expression& NewEmptyExpression();
  ast::Expression& NewInvalidExpression(const ast::Token& token,
                                        ErrorCode error_code);
  ast::Expression& NewInvalidExpression(ErrorCode error_code);
  ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  ast::Expression& NewUnaryExpression(const ast::Token& op,
                                      const ast::Expression& expression);

  // The entry of parsing an expression.
  ast::Expression& ParseExpression();

  // Helper function for parsing an expression enclosed by parenthesis for
  // do-while, if, switch, and while statements.
  const ast::Expression& ParseParenthesisExpression();

  ast::RegExp& ParseRegExp();

  std::vector<const ast::Expression*> ParseArgumentList();
  ast::Expression& ParseArrayLiteralExpression();
  ast::Expression& ParseAssignmentExpression();
  ast::Expression& ParseBinaryExpression(OperatorPrecedence category);
  ast::Expression& ParseCommaExpression();
  ast::Expression& ParseConditionalExpression();
  ast::Expression& ParseFunctionExpression(ast::FunctionKind kind);
  ast::Expression& ParseLeftHandSideExpression();
  ast::Expression& ParseMethodExpression(ast::MethodKind is_static,
                                         ast::FunctionKind kind);
  ast::Expression& ParseNameAsExpression();
  ast::Expression& ParseNewExpression();
  ast::Expression& ParseObjectLiteralExpression();
  ast::Expression& ParseParenthesis();
  ast::Expression& ParsePrimaryExpression();
  ast::Expression& ParsePropertyAfterName(ast::Expression* property_name,
                                          ast::MethodKind is_static,
                                          ast::FunctionKind function_kind);
  ast::Expression& ParseRegExpLiteral();
  ast::Expression& ParseUnaryExpression();
  ast::Expression& ParseUpdateExpression();
  ast::Expression& ParseYieldExpression();

  // Statements
  ast::Statement& HandleLabeledStatement(const ast::Name* name);

  ast::Statement& NewInvalidStatement(ErrorCode error_code);
  ast::Statement& NewEmptyStatement(const SourceCodeRange& range);

  ast::Statement& ParseStatement();

  ast::Statement& ParseBreakStatement();
  ast::Statement& ParseBlockStatement();
  ast::Statement& ParseCaseClause();
  ast::Statement& ParseClassStatement();
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
  ast::Statement& ParseNameAsStatement();
  ast::Statement& ParseReturnStatement();
  ast::Statement& ParseSwitchStatement();
  ast::Statement& ParseThrowStatement();
  ast::Statement& ParseTryStatement();
  ast::Statement& ParseVarStatement();
  ast::Statement& ParseWhileStatement();
  ast::Statement& ParseWithStatement();

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

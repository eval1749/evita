// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <vector>

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/declarations.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace internal {

//
// Parser::ExpectSemiColonScope
//
class Parser::ExpectSemiColonScope final {
 public:
  explicit ExpectSemiColonScope(Parser* parser) : parser_(parser) {}

  ~ExpectSemiColonScope() {
    parser_->ExpectToken(ast::PunctuatorKind::SemiColon,
                         ErrorCode::ERROR_STATEMENT_EXPECT_SEMI_COLON);
  }

 private:
  Parser* const parser_;

  DISALLOW_COPY_AND_ASSIGN(ExpectSemiColonScope);
};

//
// Functions for parsing statements
//
ast::Statement& Parser::NewInvalidStatement(ErrorCode error_code) {
  auto& token = ComputeInvalidToken(error_code);
  AddError(token, error_code);
  return node_factory().NewInvalidStatement(token,
                                            static_cast<int>(error_code));
}

ast::Statement& Parser::ParseStatement() {
  if (!HasToken())
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
  const auto& token = PeekToken();
  if (auto* name = token.TryAs<ast::Name>()) {
    if (name->IsKeyword())
      return ParseKeywordStatement();
    ConsumeToken();
    if (ConsumeTokenIf(ast::PunctuatorKind::Colon))
      return node_factory().NewLabeledStatement(*name, ParseStatement());
    auto& token2 = ConsumeToken();
    PushBackToken(token2);
    PushBackToken(*name);
    return ParseExpressionStatement();
  }
  if (token.Is<ast::Literal>())
    return ParseExpressionStatement();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseBlockStatement();
  if (token == ast::PunctuatorKind::SemiColon) {
    return node_factory().NewEmptyStatement(
        ConsumeToken().As<ast::Punctuator>());
  }
  return ParseExpressionStatement();
}

ast::Statement& Parser::ParseBlockStatement() {
  auto& block =
      node_factory().NewBlockStatement(ConsumeToken().As<ast::Punctuator>());
  while (HasToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    auto& token = PeekToken();
    if (token.Is<ast::Comment>()) {
      ast::NodeEditor().AppendChild(&block, &ConsumeToken());
      continue;
    }
    if (token.Is<ast::Invalid>()) {
      // TODO(eval1749): We should skip tokens until good point to restart
      // toplevel parsing.
      Advance();
      continue;
    }
    ast::NodeEditor().AppendChild(&block, &ParseStatement());
  }
  return block;
}

ast::Statement& Parser::ParseBreakStatement() {
  ConsumeToken();
  ExpectSemiColonScope semi_colon_scope(this);
  auto& label = HasToken() && PeekToken().Is<ast::Name>() ? ConsumeToken()
                                                          : NewEmptyName();
  return node_factory().NewBreakStatement(GetSourceCodeRange(), label);
}

ast::Statement& Parser::ParseCaseClause() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  DCHECK_EQ(keyword, ast::NameId::Case);
  auto& expression = ParseExpression();
  ExpectToken(ast::PunctuatorKind::Colon,
              ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  return node_factory().NewCaseClause(keyword, expression, ParseStatement());
}

ast::Statement& Parser::ParseConstStatement() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseContinueStatement() {
  ConsumeToken();
  ExpectSemiColonScope semi_colon_scope(this);
  auto& label = HasToken() && PeekToken().Is<ast::Name>() ? ConsumeToken()
                                                          : NewEmptyName();
  return node_factory().NewContinueStatement(GetSourceCodeRange(), label);
}

ast::Statement& Parser::ParseDefaultLabel() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  DCHECK(keyword == ast::NameId::Default);
  ExpectToken(ast::PunctuatorKind::Colon,
              ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  return node_factory().NewLabeledStatement(keyword, ParseStatement());
}

ast::Statement& Parser::ParseDoStatement() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  auto& statement = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::While))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_WHILE);
  ExpectSemiColonScope semi_colon_scope(this);
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  return node_factory().NewDoStatement(keyword, statement, condition);
}

ast::Statement& Parser::ParseExpressionStatement() {
  ExpectSemiColonScope semi_colon_scope(this);
  return node_factory().NewExpressionStatement(ParseExpression());
}

ast::Statement& Parser::ParseForStatement() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseFunctionStatement(ast::FunctionKind kind) {
  return node_factory().NewDeclarationStatement(ParseFunction(kind));
}

ast::Statement& Parser::ParseIfStatement() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  DCHECK_EQ(keyword, ast::NameId::If);
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  auto& then_clause = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Else))
    return node_factory().NewIfStatement(keyword, condition, then_clause);
  auto& else_clause = ParseStatement();
  return node_factory().NewIfStatement(keyword, condition, then_clause,
                                       else_clause);
}

ast::Statement& Parser::ParseKeywordStatement() {
  SourceCodeRangeScope scope(this);
  const auto& keyword = PeekToken().As<ast::Name>();
  DCHECK(keyword.IsKeyword()) << keyword;
  switch (static_cast<ast::NameId>(keyword.number())) {
    case ast::NameId::Async:
      ConsumeToken();
      ExpectToken(ast::NameId::Function,
                  ErrorCode::ERROR_FUNCTION_EXPECT_FUNCTION);
      return ParseFunctionStatement(ast::FunctionKind::Async);
    case ast::NameId::Break:
      return ParseBreakStatement();
    case ast::NameId::Case:
      return ParseCaseClause();
    case ast::NameId::Continue:
      return ParseContinueStatement();
    case ast::NameId::Const:
      return ParseConstStatement();
    case ast::NameId::Default:
      return ParseDefaultLabel();
    case ast::NameId::Do:
      return ParseDoStatement();
    case ast::NameId::For:
      return ParseForStatement();
    case ast::NameId::Function:
      ConsumeToken();
      if (ConsumeTokenIf(ast::PunctuatorKind::Times))
        return ParseFunctionStatement(ast::FunctionKind::Generator);
      return ParseFunctionStatement(ast::FunctionKind::Normal);
    case ast::NameId::If:
      return ParseIfStatement();
    case ast::NameId::Let:
      return ParseLetStatement();
    case ast::NameId::Return:
      return ParseReturnStatement();
    case ast::NameId::Switch:
      return ParseSwitchStatement();
    case ast::NameId::Throw:
      return ParseThrowStatement();
    case ast::NameId::Try:
      return ParseTryStatement();
    case ast::NameId::Var:
      return ParseVarStatement();
    case ast::NameId::While:
      return ParseWhileStatement();

    // Reserved keywords
    case ast::NameId::Catch:
    case ast::NameId::Else:
    case ast::NameId::Enum:
    case ast::NameId::Finally:
    case ast::NameId::Interface:
    case ast::NameId::Package:
    case ast::NameId::Private:
    case ast::NameId::Protected:
    case ast::NameId::Public:
    case ast::NameId::Static:
    case ast::NameId::With:
      ConsumeToken();
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
  }
  return ParseExpressionStatement();
}

ast::Statement& Parser::ParseLetStatement() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseReturnStatement() {
  ExpectSemiColonScope semi_colon_scope(this);
  ConsumeToken();
  auto& expression = HasToken() && PeekToken() == ast::PunctuatorKind::SemiColon
                         ? NewElisionExpression()
                         : ParseExpression();
  return node_factory().NewReturnStatement(GetSourceCodeRange(), expression);
}

ast::Statement& Parser::ParseSwitchStatement() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
  auto& expression = ParseExpression();
  if (!ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  std::vector<ast::Statement*> clauses;
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftBrace)) {
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_LBRACE);
    return node_factory().NewSwitchStatement(keyword, expression, clauses);
  }
  while (HasToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      return node_factory().NewSwitchStatement(keyword, expression, clauses);
    clauses.push_back(&ParseStatement());
  }
  AddError(ComputeInvalidToken(ErrorCode::ERROR_STATEMENT_EXPECT_RBRACE),
           ErrorCode::ERROR_STATEMENT_EXPECT_RBRACE);
  return node_factory().NewSwitchStatement(keyword, expression, clauses);
}

ast::Statement& Parser::ParseThrowStatement() {
  ExpectSemiColonScope semi_colon_scope(this);
  auto& keyword = ConsumeToken().As<ast::Name>();
  auto& expression = ParseExpression();
  return node_factory().NewThrowStatement(keyword, expression);
}

ast::Statement& Parser::ParseTryStatement() {
  ConsumeToken();
  auto& try_block = ParseStatement();
  if (ConsumeTokenIf(ast::NameId::Finally)) {
    auto& finally_block = ParseBlockStatement();
    return node_factory().NewTryFinallyStatement(GetSourceCodeRange(),
                                                 try_block, finally_block);
  }
  if (!ConsumeTokenIf(ast::NameId::Catch))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_CATCH);
  auto& catch_parameter = ParseExpression();
  auto& catch_block = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Finally)) {
    return node_factory().NewTryCatchStatement(GetSourceCodeRange(), try_block,
                                               catch_parameter, catch_block);
  }
  auto& finally_block = ParseStatement();
  return node_factory().NewTryCatchFinallyStatement(GetSourceCodeRange(),
                                                    try_block, catch_parameter,
                                                    catch_block, finally_block);
}

ast::Statement& Parser::ParseVarStatement() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseWhileStatement() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  auto& statement = ParseStatement();
  return node_factory().NewWhileStatement(keyword, condition, statement);
}

}  // namespace internal
}  // namespace joana

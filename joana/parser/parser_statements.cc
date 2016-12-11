// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace internal {

//
// StatementScope
//
class Parser::StatementScope final {
 public:
  StatementScope(Parser* parser, const ast::Node& keyword);
  ~StatementScope();

  ast::Name& keyword() const { return *keyword_; }

 private:
  ast::Name* keyword_;
  StatementScope* const outer_;
  Parser* const parser_;

  DISALLOW_COPY_AND_ASSIGN(StatementScope);
};

Parser::StatementScope::StatementScope(Parser* parser, const ast::Node& keyword)
    : keyword_(const_cast<ast::Name*>(&keyword.As<ast::Name>())),
      outer_(parser->statement_scope_),
      parser_(parser) {
  DCHECK(keyword.As<ast::Name>().IsKeyword()) << keyword;
}

Parser::StatementScope::~StatementScope() {
  parser_->statement_scope_ = outer_;
}

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
      return ParseStatementKeyword();
    return ParseStatementExpression();
  }
  if (token.Is<ast::Literal>())
    return ParseStatementExpression();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseStatementBlock();
  if (token == ast::PunctuatorKind::SemiColon) {
    return node_factory().NewEmptyStatement(
        ConsumeToken().As<ast::Punctuator>());
  }
  return ParseStatementExpression();
}

ast::Statement& Parser::ParseStatementAsync() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementBlock() {
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

ast::Statement& Parser::ParseStatementBreak() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementConst() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementContinue() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementDo() {
  StatementScope do_scope(this, ConsumeToken());
  auto& statement = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::While))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_DO_EXPECT_WHILE);
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_DO_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  if (!ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_DO_EXPECT_RPAREN);
  if (!ConsumeTokenIf(ast::PunctuatorKind::SemiColon))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_DO_EXPECT_SEMI_COLON);
  return node_factory().NewDoWhileStatement(do_scope.keyword(), statement,
                                            condition);
}

ast::Statement& Parser::ParseStatementExpression() {
  auto& result = node_factory().NewExpressionStatement(ParseExpression());
  if (ConsumeTokenIf(ast::PunctuatorKind::SemiColon))
    return result;
  AddError(lexer_->location(), ErrorCode::ERROR_STATEMENT_EXPECT_SEMI_COLON);
  return result;
}

ast::Statement& Parser::ParseStatementFor() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementFunction() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementIf() {
  auto& if_keyword = ConsumeToken().As<ast::Name>();
  DCHECK_EQ(if_keyword, ast::NameId::If);
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_IF_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  if (!ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_IF_EXPECT_RPAREN);
  auto& then_clause = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Else))
    return node_factory().NewIfStatement(if_keyword, condition, then_clause);
  auto& else_clause = ParseStatement();
  return node_factory().NewIfStatement(if_keyword, condition, then_clause,
                                       else_clause);
}

ast::Statement& Parser::ParseStatementKeyword() {
  const auto& keyword = PeekToken().As<ast::Name>();
  DCHECK(keyword.IsKeyword()) << keyword;
  switch (static_cast<ast::NameId>(keyword.number())) {
    case ast::NameId::Async:
      return ParseStatementAsync();
    case ast::NameId::Await:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_AWAIT);
    case ast::NameId::Break:
      return ParseStatementBreak();
    case ast::NameId::Case:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_CASE);
    case ast::NameId::Continue:
      return ParseStatementContinue();
    case ast::NameId::Const:
      return ParseStatementConst();
    case ast::NameId::Debugger:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
    case ast::NameId::Do:
      return ParseStatementDo();
    case ast::NameId::Else:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_ELSE);
    case ast::NameId::Finally:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_FINALLY);
    case ast::NameId::For:
      return ParseStatementFor();
    case ast::NameId::Function:
      return ParseStatementFunction();
    case ast::NameId::If:
      return ParseStatementIf();
    case ast::NameId::Let:
      return ParseStatementLet();
    case ast::NameId::Return:
      return ParseStatementReturn();
    case ast::NameId::Switch:
      return ParseStatementSwitch();
    case ast::NameId::Throw:
      return ParseStatementThrow();
    case ast::NameId::Var:
      return ParseStatementVar();
    case ast::NameId::While:
      return ParseStatementWhile();
    case ast::NameId::Yield:
      return ParseStatementYield();

    // Reserved keywords
    case ast::NameId::Enum:
    case ast::NameId::Interface:
    case ast::NameId::Package:
    case ast::NameId::Private:
    case ast::NameId::Protected:
    case ast::NameId::Public:
    case ast::NameId::Static:
    case ast::NameId::With:
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
  }
  return ParseStatementExpression();
}

ast::Statement& Parser::ParseStatementLet() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementReturn() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementSwitch() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementThrow() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementVar() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementWhile() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementYield() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

}  // namespace internal
}  // namespace joana

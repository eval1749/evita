// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace internal {

ast::Statement& Parser::NewInvalidStatement(const ast::Node& node,
                                            ErrorCode error_code) {
  AddError(node, error_code);
  return node_factory().NewInvalidStatement(node, static_cast<int>(error_code));
}

ast::Statement& Parser::ParseStatement() {
  const auto& token = GetToken();
  if (auto* name = token.TryAs<ast::Name>()) {
    if (name->IsKeyword())
      return ParseStatementKeyword();
    return ParseStatementExpression();
  }
  if (token.Is<ast::Literal>())
    return ParseStatementExpression();
  if (auto* punctator = token.TryAs<ast::Punctuator>()) {
    if (punctator->kind() == ast::PunctuatorKind::SemiColon) {
      Advance();
      return node_factory().NewEmptyStatement(*punctator);
    }
    return ParseStatementExpression();
  }
  Advance();
  return NewInvalidStatement(token, ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementAsync() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementBreak() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementConst() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementContinue() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementDo() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementExpression() {
  auto& result = node_factory().NewExpressionStatement(ParseExpression());
  if (AdvanceIf(ast::PunctuatorKind::SemiColon))
    return result;
  AddError(lexer_->location(), ErrorCode::ERROR_STATEMENT_EXPECT_SEMI_COLON);
  return result;
}

ast::Statement& Parser::ParseStatementFor() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementFunction() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementIf() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementKeyword() {
  const auto& keyword = GetToken().As<ast::Name>();
  DCHECK(keyword.IsKeyword());
  switch (static_cast<ast::NameId>(keyword.number())) {
    case ast::NameId::Async:
      return ParseStatementAsync();
    case ast::NameId::Await:
      return NewInvalidStatement(keyword, ErrorCode::ERROR_STATEMENT_AWAIT);
    case ast::NameId::Break:
      return ParseStatementBreak();
    case ast::NameId::Case:
      return NewInvalidStatement(keyword, ErrorCode::ERROR_STATEMENT_CASE);
    case ast::NameId::Continue:
      return ParseStatementContinue();
    case ast::NameId::Const:
      return ParseStatementConst();
    case ast::NameId::Debugger:
      return NewInvalidStatement(keyword,
                                 ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
    case ast::NameId::Do:
      return ParseStatementDo();
    case ast::NameId::Else:
      return NewInvalidStatement(keyword, ErrorCode::ERROR_STATEMENT_ELSE);
    case ast::NameId::Finally:
      return NewInvalidStatement(keyword, ErrorCode::ERROR_STATEMENT_FINALLY);
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
      return NewInvalidStatement(keyword,
                                 ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
  }
  return ParseStatementExpression();
}

ast::Statement& Parser::ParseStatementLet() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementReturn() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementSwitch() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementThrow() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementVar() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementWhile() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementYield() {
  return NewInvalidStatement(GetToken(), ErrorCode::ERROR_STATEMENT_INVALID);
}

}  // namespace internal
}  // namespace joana

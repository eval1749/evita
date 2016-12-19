// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <vector>

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/declarations.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/source_code.h"

namespace joana {
namespace internal {

namespace {

bool IsDeclarationKeyword(const ast::Token& token) {
  auto* const name = token.TryAs<ast::Name>();
  if (!name)
    return false;
  return *name == ast::NameId::Const || *name == ast::NameId::Let ||
         *name == ast::NameId::Var;
}

}  // namespace

//
// Parser::ExpectSemicolonScope
//
class Parser::ExpectSemicolonScope final {
 public:
  explicit ExpectSemicolonScope(Parser* parser) : parser_(parser) {}

  ~ExpectSemicolonScope() { parser_->ExpectSemicolon(); }

 private:
  Parser* const parser_;

  DISALLOW_COPY_AND_ASSIGN(ExpectSemicolonScope);
};

//
// Functions for parsing statements
//

// Called after before consuming ':'
ast::Statement& Parser::HandleLabeledStatement(const ast::Name* label) {
  auto& colon = ConsumeToken();
  DCHECK_EQ(colon, ast::PunctuatorKind::Colon);
  if (!options_.disable_automatic_semicolon) {
    if (!CanPeekToken() || PeekToken() == ast::PunctuatorKind::RightBrace) {
      auto& statement =
          NewEmptyStatement(SourceCodeRange::CollapseToEnd(colon.range()));
      return node_factory().NewLabeledStatement(GetSourceCodeRange(), *label,
                                                statement);
    }
  }
  auto& statement = ParseStatement();
  return node_factory().NewLabeledStatement(GetSourceCodeRange(), *label,
                                            statement);
}

ast::Statement& Parser::NewEmptyStatement(const SourceCodeRange& range) {
  DCHECK(range.IsCollapsed()) << range;
  return node_factory().NewEmptyStatement(range);
}

ast::Statement& Parser::NewInvalidStatement(ErrorCode error_code) {
  auto& token = ComputeInvalidToken(error_code);
  AddError(GetSourceCodeRange(), error_code);
  return node_factory().NewInvalidStatement(token,
                                            static_cast<int>(error_code));
}

ast::Statement& Parser::ParseBlockStatement() {
  SourceCodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBrace);
  ConsumeToken();
  std::vector<ast::Statement*> statements;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    auto& token = PeekToken();
    if (token.Is<ast::Comment>()) {
      Advance();
      continue;
    }
    if (token.Is<ast::Invalid>()) {
      // TODO(eval1749): We should skip tokens until good point to restart
      // toplevel parsing.
      Advance();
      continue;
    }
    statements.push_back(&ParseStatement());
  }
  return node_factory().NewBlockStatement(GetSourceCodeRange(), statements);
}

ast::Statement& Parser::ParseBreakStatement() {
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon)
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewBreakStatement(GetSourceCodeRange(),
                                            NewEmptyName());
  }
  ExpectSemicolonScope semicolon_scope(this);
  auto& label = CanPeekToken() && PeekToken().Is<ast::Name>() ? ConsumeToken()
                                                              : NewEmptyName();
  return node_factory().NewBreakStatement(GetSourceCodeRange(), label);
}

ast::Statement& Parser::ParseCaseClause() {
  SourceCodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::NameId::Case);
  ConsumeToken();
  auto& expression = ParseExpression();
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::Colon) {
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  } else {
    auto& colon = ConsumeToken();
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightBrace) {
      if (options_.disable_automatic_semicolon)
        AddError(ErrorCode::ERROR_STATEMENT_EXPECT_SEMICOLON);
      return node_factory().NewCaseClause(
          GetSourceCodeRange(), expression,
          NewEmptyStatement(SourceCodeRange::CollapseToEnd(colon.range())));
    }
  }
  auto& statement = ParseStatement();
  return node_factory().NewCaseClause(GetSourceCodeRange(), expression,
                                      statement);
}

ast::Statement& Parser::ParseClassStatement() {
  return node_factory().NewDeclarationStatement(ParseClass());
}

ast::Statement& Parser::ParseConstStatement() {
  ExpectSemicolonScope semicolon_scope(this);
  ConsumeToken();
  auto& expression = ParseExpression();
  return node_factory().NewConstStatement(GetSourceCodeRange(), expression);
}

ast::Statement& Parser::ParseContinueStatement() {
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon)
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewContinueStatement(GetSourceCodeRange(),
                                               NewEmptyName());
  }
  ExpectSemicolonScope semicolon_scope(this);
  auto& label = CanPeekToken() && PeekToken().Is<ast::Name>() ? ConsumeToken()
                                                              : NewEmptyName();
  return node_factory().NewContinueStatement(GetSourceCodeRange(), label);
}

ast::Statement& Parser::ParseDefaultLabel() {
  SourceCodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::NameId::Default);
  auto& label = ConsumeToken().As<ast::Name>();
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::Colon)
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  return HandleLabeledStatement(&label);
}

ast::Statement& Parser::ParseDoStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Do);
  ConsumeToken();
  auto& statement = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::While))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_WHILE);
  if (options_.disable_automatic_semicolon) {
    ExpectSemicolonScope semicolon_scope(this);
    auto& condition = ParseParenthesisExpression();
    return node_factory().NewDoStatement(GetSourceCodeRange(), statement,
                                         condition);
  }
  auto& condition = ParseParenthesisExpression();
  ConsumeTokenIf(ast::PunctuatorKind::Semicolon);
  return node_factory().NewDoStatement(GetSourceCodeRange(), statement,
                                       condition);
}

ast::Statement& Parser::ParseExpressionStatement() {
  auto& expression = ParseExpression();
  if (auto* decl_expr = expression.TryAs<ast::DeclarationExpression>()) {
    auto& declaration = decl_expr->declaration();
    if (!declaration.Is<ast::ArrowFunction>())
      return node_factory().NewDeclarationStatement(declaration);
  }
  ExpectSemicolonScope semicolon_scope(this);
  return node_factory().NewExpressionStatement(expression);
}

ast::Statement& Parser::ParseForStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::For);
  ConsumeToken();
  ExpectPunctuator(ast::PunctuatorKind::LeftParenthesis,
                   ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);

  auto& keyword = CanPeekToken() && IsDeclarationKeyword(PeekToken())
                      ? ConsumeToken()
                      : NewEmptyName();
  auto& expression =
      CanPeekToken() && PeekToken() == ast::PunctuatorKind::Semicolon
          ? NewElisionExpression()
          : ParseExpression();

  if (ConsumeTokenIf(ast::PunctuatorKind::Semicolon)) {
    // 'for' '(' binding ';' condition ';' step ')' statement
    auto& condition =
        CanPeekToken() && PeekToken() == ast::PunctuatorKind::Semicolon
            ? NewElisionExpression()
            : ParseExpression();
    ExpectPunctuator(ast::PunctuatorKind::Semicolon,
                     ErrorCode::ERROR_STATEMENT_EXPECT_SEMICOLON);
    auto& step =
        CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightParenthesis
            ? NewElisionExpression()
            : ParseExpression();
    ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                     ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
    auto& body = ParseStatement();
    return node_factory().NewForStatement(GetSourceCodeRange(), keyword,
                                          expression, condition, step, body);
  }

  if (ConsumeTokenIf(ast::NameId::Of)) {
    // 'for' '(' binding 'of' expression ')' statement
    auto& expression2 = ParseAssignmentExpression();
    ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                     ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
    auto& body = ParseStatement();
    return node_factory().NewForOfStatement(GetSourceCodeRange(), keyword,
                                            expression, expression2, body);
  }

  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis)) {
    // 'for' '(' binding 'in' expression ')' statement
    auto& body = ParseStatement();
    return node_factory().NewForInStatement(GetSourceCodeRange(), keyword,
                                            expression, body);
  }

  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseFunctionStatement(ast::FunctionKind kind) {
  return node_factory().NewDeclarationStatement(ParseFunction(kind));
}

ast::Statement& Parser::ParseIfStatement() {
  auto& keyword = ConsumeToken().As<ast::Name>();
  DCHECK_EQ(keyword, ast::NameId::If);
  auto& condition = ParseParenthesisExpression();
  auto& then_clause = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Else)) {
    return node_factory().NewIfStatement(GetSourceCodeRange(), condition,
                                         then_clause);
  }
  auto& else_clause = ParseStatement();
  return node_factory().NewIfElseStatement(GetSourceCodeRange(), condition,
                                           then_clause, else_clause);
}

ast::Statement& Parser::ParseKeywordStatement() {
  SourceCodeRangeScope scope(this);
  const auto& keyword = PeekToken().As<ast::Name>();
  DCHECK(keyword.IsKeyword()) << keyword;
  switch (static_cast<ast::NameId>(keyword.number())) {
    case ast::NameId::Async:
      ConsumeToken();
      if (CanPeekToken() && PeekToken() == ast::NameId::Function)
        return ParseFunctionStatement(ast::FunctionKind::Async);
      PushBackToken(keyword);
      break;
    case ast::NameId::Break:
      return ParseBreakStatement();
    case ast::NameId::Case:
      return ParseCaseClause();
    case ast::NameId::Class:
      return ParseClassStatement();
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

    case ast::NameId::With:
      return ParseWithStatement();

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
      ConsumeToken();
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
  }
  return ParseExpressionStatement();
}

ast::Statement& Parser::ParseLetStatement() {
  ExpectSemicolonScope semicolon_scope(this);
  ConsumeToken();
  auto& expression = ParseExpression();
  return node_factory().NewLetStatement(GetSourceCodeRange(), expression);
}

ast::Statement& Parser::ParseNameAsStatement() {
  auto& name = PeekToken().As<ast::Name>();
  if (name.IsKeyword())
    return ParseKeywordStatement();
  ConsumeToken();
  if (!CanPeekToken()) {
    PushBackToken(name);
    return ParseExpressionStatement();
  }
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon)
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
  } else {
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::Colon)
      return HandleLabeledStatement(&name);
  }
  PushBackToken(name);
  return ParseExpressionStatement();
}

// Yet another entry point called by statement parser.
const ast::Expression& Parser::ParseParenthesisExpression() {
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis)) {
    // Some people think it is redundant that C++ statement requires
    // parenthesis for an expression after keyword.
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
    return ParseExpression();
  }
  auto& expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                   ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  return expression;
}

ast::Statement& Parser::ParseReturnStatement() {
  ExpectSemicolonScope semicolon_scope(this);
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon)
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                             NewElisionExpression());
  } else {
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightBrace) {
      return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                               NewElisionExpression());
    }
  }
  if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::Semicolon) {
    return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                             NewElisionExpression());
  }
  auto& expression = ParseExpression();
  return node_factory().NewReturnStatement(GetSourceCodeRange(), expression);
}

// The entry point
ast::Statement& Parser::ParseStatement() {
  if (!CanPeekToken())
    return NewEmptyStatement(source_code().end());
  SourceCodeRangeScope scope(this);
  const auto& token = PeekToken();
  if (token.Is<ast::Name>())
    return ParseNameAsStatement();
  if (token.Is<ast::Literal>())
    return ParseExpressionStatement();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseBlockStatement();
  if (token == ast::PunctuatorKind::Semicolon) {
    auto& statement = NewEmptyStatement(
        SourceCodeRange::CollapseToStart(PeekToken().range()));
    ConsumeToken();
    return statement;
  }
  return ParseExpressionStatement();
}

ast::Statement& Parser::ParseSwitchStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Switch);
  ConsumeToken();
  auto& expression = ParseParenthesisExpression();
  std::vector<ast::Statement*> clauses;
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftBrace)) {
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_LBRACE);
    return node_factory().NewSwitchStatement(GetSourceCodeRange(), expression,
                                             clauses);
  }
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    clauses.push_back(&ParseStatement());
  }
  return node_factory().NewSwitchStatement(GetSourceCodeRange(), expression,
                                           clauses);
}

ast::Statement& Parser::ParseThrowStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Throw);
  ConsumeToken();
  ExpectSemicolonScope semicolon_scope(this);
  auto& expression = ParseExpression();
  return node_factory().NewThrowStatement(GetSourceCodeRange(), expression);
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
  ExpectSemicolonScope semicolon_scope(this);
  ConsumeToken();
  auto& expression = ParseExpression();
  return node_factory().NewVarStatement(GetSourceCodeRange(), expression);
}

ast::Statement& Parser::ParseWhileStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::While);
  ConsumeToken();
  auto& condition = ParseParenthesisExpression();
  auto& statement = ParseStatement();
  return node_factory().NewWhileStatement(GetSourceCodeRange(), condition,
                                          statement);
}

ast::Statement& Parser::ParseWithStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::With);
  ConsumeToken();
  auto& expression = ParseParenthesisExpression();
  auto& statement = ParseStatement();
  return node_factory().NewWithStatement(GetSourceCodeRange(), expression,
                                         statement);
}

}  // namespace internal
}  // namespace joana

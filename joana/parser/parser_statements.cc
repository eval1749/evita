// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

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

namespace {

bool IsLoopKeyword(const ast::Name& keyword) {
  return keyword == ast::NameId::Do || keyword == ast::NameId::For ||
         keyword == ast::NameId::While;
}

}  // namespace

//
// StatementScope
//
class Parser::StatementScope final {
 public:
  class Scopes final {
   public:
    class Iterator final
        : public std::iterator<std::input_iterator_tag, StatementScope> {
     public:
      Iterator(const Iterator& other)
          : owner_(other.owner_), scope_(other.scope_) {}

      ~Iterator() = default;

      reference operator*() const {
        DCHECK(scope_);
        return *scope_;
      }

      Iterator& operator++() {
        scope_ = scope_->outer_;
        return *this;
      }

      bool operator==(const Iterator& other) const {
        DCHECK_EQ(owner_, other.owner_);
        return scope_ == other.scope_;
      }

      bool operator!=(const Iterator& other) const {
        return !operator==(other);
      }

     private:
      friend class Scopes;

      Iterator(const Scopes* owner, StatementScope* scope)
          : owner_(owner), scope_(scope) {}

      const Scopes* owner_;
      StatementScope* scope_;
    };

    explicit Scopes(StatementScope* scope) : start_(scope) {}
    Scopes(const Scopes& other) : start_(other.start_) {}
    ~Scopes() = default;

    Iterator begin() { return Iterator(this, start_); }
    Iterator end() { return Iterator(this, nullptr); }

   private:
    StatementScope* start_;
  };

  StatementScope(Parser* parser, const ast::Node& keyword);
  ~StatementScope();

  ast::Name& keyword() const { return *keyword_; }
  static Scopes ScopesOf(StatementScope* start) { return Scopes(start); }

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
  parser_->statement_scope_ = this;
}

Parser::StatementScope::~StatementScope() {
  parser_->statement_scope_ = outer_;
}

//
// Functions for parsing statements
//
bool Parser::CanUseBreak() const {
  for (const auto& scope : StatementScope::ScopesOf(statement_scope_)) {
    auto& keyword = scope.keyword();
    if (IsLoopKeyword(keyword) || keyword == ast::NameId::Switch)
      return true;
    if (keyword == ast::NameId::Finally)
      return false;
  }
  return false;
}

bool Parser::CanUseContinue() const {
  for (const auto& scope : StatementScope::ScopesOf(statement_scope_)) {
    auto& keyword = scope.keyword();
    if (IsLoopKeyword(keyword))
      return true;
    if (keyword == ast::NameId::Finally)
      return false;
  }
  return false;
}

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
  auto& break_keyword = ConsumeToken().As<ast::Name>();
  if (!CanUseBreak())
    AddError(break_keyword, ErrorCode::ERROR_STATEMENT_BREAK_BAD_PLACE);
  if (!HasToken())
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
  if (ConsumeTokenIf(ast::PunctuatorKind::SemiColon))
    return node_factory().NewBreakStatement(break_keyword);
  if (!PeekToken().Is<ast::Name>())
    AddError(PeekToken(), ErrorCode::ERROR_STATEMENT_BREAK_NOT_LABEL);
  auto& label = ConsumeToken().As<ast::Name>();
  ExpectToken(ast::PunctuatorKind::SemiColon,
              ErrorCode::ERROR_STATEMENT_BREAK_SEMI_COLON);
  // TODO(eval1749): Find label for |break| statement
  return node_factory().NewBreakStatement(break_keyword, label);
}

ast::Statement& Parser::ParseStatementConst() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementContinue() {
  auto& continue_keyword = ConsumeToken().As<ast::Name>();
  if (!CanUseContinue())
    AddError(continue_keyword, ErrorCode::ERROR_STATEMENT_CONTINUE_BAD_PLACE);
  if (!HasToken())
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
  if (ConsumeTokenIf(ast::PunctuatorKind::SemiColon))
    return node_factory().NewContinueStatement(continue_keyword);
  if (!PeekToken().Is<ast::Name>())
    AddError(PeekToken(), ErrorCode::ERROR_STATEMENT_CONTINUE_NOT_LABEL);
  auto& label = ConsumeToken().As<ast::Name>();
  ExpectToken(ast::PunctuatorKind::SemiColon,
              ErrorCode::ERROR_STATEMENT_CONTINUE_SEMI_COLON);
  // TODO(eval1749): Find label for |continue| statement
  return node_factory().NewContinueStatement(continue_keyword, label);
}

ast::Statement& Parser::ParseStatementDo() {
  auto& keywod = ConsumeToken().As<ast::Name>();
  StatementScope do_scope(this, keywod);
  auto& statement = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::While))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_DO_EXPECT_WHILE);
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_DO_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_DO_EXPECT_RPAREN);
  ExpectToken(ast::PunctuatorKind::SemiColon,
              ErrorCode::ERROR_STATEMENT_DO_EXPECT_SEMI_COLON);
  return node_factory().NewDoStatement(keywod, statement, condition);
}

ast::Statement& Parser::ParseStatementExpression() {
  auto& result = node_factory().NewExpressionStatement(ParseExpression());
  ExpectToken(ast::PunctuatorKind::SemiColon,
              ErrorCode::ERROR_STATEMENT_EXPECT_SEMI_COLON);
  return result;
}

ast::Statement& Parser::ParseStatementFor() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementFunction() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementIf() {
  auto& keywod = ConsumeToken().As<ast::Name>();
  DCHECK_EQ(keywod, ast::NameId::If);
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_IF_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_IF_EXPECT_RPAREN);
  auto& then_clause = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Else))
    return node_factory().NewIfStatement(keywod, condition, then_clause);
  auto& else_clause = ParseStatement();
  return node_factory().NewIfStatement(keywod, condition, then_clause,
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
  auto& keyword = ConsumeToken().As<ast::Name>();
  auto& expression = ParseExpression();
  ExpectToken(ast::PunctuatorKind::SemiColon,
              ErrorCode::ERROR_STATEMENT_THROW_EXPECT_SEMI_COLON);
  return node_factory().NewThrowStatement(keyword, expression);
}

ast::Statement& Parser::ParseStatementVar() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

ast::Statement& Parser::ParseStatementWhile() {
  auto& keywod = ConsumeToken().As<ast::Name>();
  StatementScope while_scope(this, keywod);
  ExpectToken(ast::PunctuatorKind::LeftParenthesis,
              ErrorCode::ERROR_STATEMENT_DO_EXPECT_LPAREN);
  auto& condition = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_STATEMENT_DO_EXPECT_RPAREN);
  auto& statement = ParseStatement();
  return node_factory().NewWhileStatement(keywod, condition, statement);
}

ast::Statement& Parser::ParseStatementYield() {
  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

}  // namespace internal
}  // namespace joana

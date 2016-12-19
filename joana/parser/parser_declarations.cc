// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/statements.h"

namespace joana {
namespace internal {

ast::ArrowFunctionBody& Parser::ParseArrowFunctionBody() {
  if (!CanPeekToken()) {
    return NewInvalidStatement(
        ErrorCode::ERROR_FUNCTION_INVALID_ARROW_FUNCTION_BODY);
  }
  if (PeekToken() == ast::PunctuatorKind::LeftBrace)
    return ParseStatement();
  return ParseAssignmentExpression();
}

ast::Class& Parser::ParseClass() {
  SourceCodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::NameId::Class);
  ConsumeToken();
  auto& class_name = ParseClassName();
  auto& heritage = ParseClassHeritage();
  auto& class_body = ParseClassBody();
  return node_factory().NewClass(GetSourceCodeRange(), class_name, heritage,
                                 class_body);
}

ast::Expression& Parser::ParseClassBody() {
  SourceCodeRangeScope scope(this);
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::LeftBrace)
    return NewInvalidExpression(ErrorCode::ERROR_CLASS_EXPECT_LBRACE);
  return ParsePrimaryExpression();
}

ast::Expression& Parser::ParseClassHeritage() {
  if (!ConsumeTokenIf(ast::NameId::Extends))
    return NewEmptyExpression();
  return ParseLeftHandSideExpression();
}

ast::Token& Parser::ParseClassName() {
  if (!CanPeekToken() || !PeekToken().Is<ast::Name>())
    return NewEmptyName();
  if (PeekToken() == ast::NameId::Extends)
    return NewEmptyName();
  return ConsumeToken().As<ast::Name>();
}

ast::Function& Parser::ParseFunction(ast::FunctionKind kind) {
  auto& name = PeekToken().Is<ast::Name>() ? ConsumeToken() : NewEmptyName();
  auto& parameter_list = ParseParameterList();
  auto& body = ParseFunctionBody();
  return node_factory().NewFunction(GetSourceCodeRange(), kind, name,
                                    parameter_list, body);
}

ast::Statement& Parser::ParseFunctionBody() {
  SourceCodeRangeScope scope(this);
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::LeftBrace)
    return NewInvalidStatement(ErrorCode::ERROR_FUNCTION_EXPECT_LBRACE);
  return ParseStatement();
}

ast::Method& Parser::ParseMethod(ast::MethodKind is_static,
                                 ast::FunctionKind kind) {
  auto& method_name = ParsePropertyName();
  auto& parameter_list = ParseParameterList();
  auto& method_body = ParseFunctionBody();
  return node_factory().NewMethod(GetSourceCodeRange(), is_static, kind,
                                  method_name, parameter_list, method_body);
}

ast::Expression& Parser::ParseParameterList() {
  SourceCodeRangeScope scope(this);
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewInvalidExpression(ErrorCode::ERROR_FUNCTION_EXPECT_LPAREN);
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return NewEmptyExpression();
  auto& expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                   ErrorCode::ERROR_FUNCTION_EXPECT_RPAREN);
  return node_factory().NewGroupExpression(GetSourceCodeRange(), expression);
}

}  // namespace internal
}  // namespace joana

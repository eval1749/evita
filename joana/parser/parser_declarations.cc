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
  if (!HasToken()) {
    return NewInvalidStatement(
        ErrorCode::ERROR_FUNCTION_INVALID_ARROW_FUNCTION_BODY);
  }
  if (PeekToken() == ast::PunctuatorKind::LeftBrace)
    return ParseStatement();
  return ParseExpression();
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
  if (!HasToken() || PeekToken() != ast::PunctuatorKind::LeftBrace)
    return NewInvalidStatement(ErrorCode::ERROR_FUNCTION_EXPECT_LBRACE);
  return ParseStatement();
}

ast::Expression& Parser::ParseParameterList() {
  SourceCodeRangeScope scope(this);
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewInvalidExpression(ErrorCode::ERROR_FUNCTION_EXPECT_LPAREN);
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return node_factory().NewEmptyExpression(GetSourceCodeRange());
  auto& expression = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_FUNCTION_EXPECT_RPAREN);
  return node_factory().NewGroupExpression(GetSourceCodeRange(), expression);
}

}  // namespace internal
}  // namespace joana

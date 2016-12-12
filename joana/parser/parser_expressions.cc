// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace internal {

ast::Expression& Parser::NewInvalidExpression(ErrorCode error_code) {
  auto& token = ComputeInvalidToken(error_code);
  AddError(token, error_code);
  return node_factory().NewInvalidExpression(token,
                                             static_cast<int>(error_code));
}

ast::Expression& Parser::NewLiteralExpression(const ast::Literal& literal) {
  return node_factory().NewLiteralExpression(literal);
}

ast::Expression& Parser::ParseExpression() {
  if (!HasToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  const auto& token = PeekToken();
  if (token.Is<ast::Name>())
    return ParseExpressionAfterName(ConsumeToken().As<ast::Name>());
  if (auto* punctator = token.TryAs<ast::Punctuator>()) {
    Advance();
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  }
  Advance();
  return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
}

ast::Expression& Parser::ParseExpressionAfterName(const ast::Name& name) {
  switch (static_cast<ast::NameId>(name.number())) {
    case ast::NameId::False:
      return NewLiteralExpression(
          node_factory().NewBooleanLiteral(name, false));
    case ast::NameId::Null:
      return NewLiteralExpression(node_factory().NewNullLiteral(name));
    case ast::NameId::True:
      return NewLiteralExpression(node_factory().NewBooleanLiteral(name, true));
  }
  if (name.IsKeyword())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  return node_factory().NewReferenceExpression(name);
}

}  // namespace internal
}  // namespace joana

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

ast::Expression& Parser::NewInvalidExpression(const ast::Node& node,
                                              ErrorCode error_code) {
  AddError(node, error_code);
  return node_factory().NewInvalidExpression(node,
                                             static_cast<int>(error_code));
}

ast::Expression& Parser::NewLiteralExpression(const ast::Literal& literal) {
  return node_factory().NewLiteralExpression(literal);
}

ast::Expression& Parser::ParseExpression() {
  const auto& token = GetToken();
  if (auto* name = token.TryAs<ast::Name>())
    return ParseExpressionName();
  if (auto* punctator = token.TryAs<ast::Punctuator>()) {
    Advance();
    return NewInvalidExpression(token, ErrorCode::ERROR_EXPRESSION_NYI);
  }
  Advance();
  return NewInvalidExpression(token, ErrorCode::ERROR_EXPRESSION_NYI);
}

ast::Expression& Parser::ParseExpressionName() {
  const auto& name = *GetToken().TryAs<ast::Name>();
  switch (static_cast<ast::NameId>(name.number())) {
    case ast::NameId::False:
      Advance();
      return NewLiteralExpression(
          node_factory().NewBooleanLiteral(name, false));
    case ast::NameId::Null:
      Advance();
      return NewLiteralExpression(node_factory().NewNullLiteral(name));
    case ast::NameId::True:
      Advance();
      return NewLiteralExpression(node_factory().NewBooleanLiteral(name, true));
  }
  return NewInvalidExpression(name, ErrorCode::ERROR_EXPRESSION_NYI);
}

}  // namespace internal
}  // namespace joana

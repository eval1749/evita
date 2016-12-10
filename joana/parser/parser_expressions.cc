// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/name.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/punctuator.h"

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
  const auto& token = lexer_->GetToken();
  if (auto* name = token.TryAs<ast::Name>())
    return ParseExpressionName();
  if (auto* punctator = token.TryAs<ast::Punctuator>()) {
    lexer_->Advance();
    return NewInvalidExpression(token, ErrorCode::ERROR_EXPRESSION_NYI);
  }
  lexer_->Advance();
  return NewInvalidExpression(token, ErrorCode::ERROR_EXPRESSION_NYI);
}

ast::Expression& Parser::ParseExpressionName() {
  const auto& name = *lexer_->GetToken().TryAs<ast::Name>();
  switch (static_cast<ast::NameId>(name.number())) {
    case ast::NameId::False:
      lexer_->Advance();
      return NewLiteralExpression(
          node_factory().NewBooleanLiteral(name, false));
    case ast::NameId::True:
      lexer_->Advance();
      return NewLiteralExpression(node_factory().NewBooleanLiteral(name, true));
  }
  return NewInvalidExpression(name, ErrorCode::ERROR_EXPRESSION_NYI);
}

}  // namespace internal
}  // namespace joana

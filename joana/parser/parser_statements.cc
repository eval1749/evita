// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/punctuator.h"
#include "joana/public/ast/statements.h"

namespace joana {
namespace internal {

ast::Statement& Parser::NewInvalidStatement(const ast::Node& node,
                                            ErrorCode error_code) {
  AddError(node, error_code);
  return node_factory().NewInvalidStatement(node, static_cast<int>(error_code));
}

ast::Statement& Parser::ParseStatement() {
  const auto& token = lexer_->GetToken();
  if (auto* punctator = token.TryAs<ast::Punctuator>()) {
    if (punctator->kind() == ast::PunctuatorKind::SemiColon) {
      lexer_->Advance();
      return node_factory().NewEmptyStatement(*punctator);
    }
  }
  auto& result = node_factory().NewExpressionStatement(ParseExpression());
  if (AdvanceIf(ast::PunctuatorKind::SemiColon))
    return result;
  AddError(lexer_->location(), ErrorCode::ERROR_STATEMENT_EXPECT_SEMI_COLON);
  return result;
}

}  // namespace internal
}  // namespace joana

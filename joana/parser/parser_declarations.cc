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

namespace {

bool IsValidParameter(const ast::Expression& expression) {
  if (expression.Is<ast::ReferenceExpression>())
    return true;
  auto* const assign = expression.TryAs<ast::AssignmentExpression>();
  if (!assign || assign->op() != ast::PunctuatorKind::Equal)
    return false;
  return assign->lhs().Is<ast::ReferenceExpression>();
}

}  // namespace

ast::ArrowFunctionBody& Parser::ExpectArrowFunctionBody() {
  if (!HasToken()) {
    return NewInvalidStatement(
        ErrorCode::ERROR_FUNCTION_INVALID_ARROW_FUNCTION_BODY);
  }
  if (PeekToken() == ast::PunctuatorKind::LeftBrace)
    return ParseStatement();
  return ParseExpression();
}

std::vector<ast::Expression*> Parser::ExpectParameterList(
    const ast::Expression& expression) {
  if (IsValidParameter(expression))
    return {const_cast<ast::Expression*>(&expression)};
  if (auto* list = expression.TryAs<ast::CommaExpression>()) {
    std::vector<ast::Expression*> parameters;
    for (const auto& member : list->expressions().elements()) {
      if (!IsValidParameter(*member)) {
        AddError(expression, ErrorCode::ERROR_FUNCTION_INVALID_PARAMETER);
        continue;
      }
      parameters.push_back(member);
    }
    return parameters;
  }
  AddError(expression, ErrorCode::ERROR_FUNCTION_INVALID_PARAMETER_LIST);
  return {};
}

}  // namespace internal
}  // namespace joana

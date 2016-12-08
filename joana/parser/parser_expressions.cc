// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/punctuator.h"

namespace joana {
namespace internal {

void Parser::ParseExpression() {
  const auto& token = lexer_->GetToken();
  if (auto* punctator = token.as<ast::Punctuator>()) {
    AddError(token, ErrorCode::ERROR_EXPRESSION_NYI);
    return;
  }
  AddError(token, ErrorCode::ERROR_EXPRESSION_NYI);
}

}  // namespace internal
}  // namespace joana

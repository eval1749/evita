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

void Parser::ParseStatement() {
  const auto& token = lexer_->GetToken();
  if (auto* punctator = token.as<ast::Punctuator>()) {
    if (punctator->kind() == ast::PunctuatorKind::SemiColon)
      return AddStatement(node_factory().NewEmptyStatement(*punctator));
  }
  AddError(token, ErrorCode::ERROR_STATEMENT_NYI);
}

}  // namespace internal
}  // namespace joana

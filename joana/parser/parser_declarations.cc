// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/parser.h"

#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/parser/parser_error_codes.h"

namespace joana {
namespace parser {

const ast::Node& Parser::ParseArrowFunctionBody() {
  if (!CanPeekToken()) {
    return NewInvalidStatement(
        ErrorCode::ERROR_FUNCTION_INVALID_ARROW_FUNCTION_BODY);
  }
  if (PeekToken() == ast::TokenKind::LeftBrace)
    return ParseStatement();
  return ParseAssignmentExpression();
}

const ast::Node& Parser::ParseClass() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::TokenKind::Class);
  ConsumeToken();
  auto& class_name = ParseClassName();
  auto& heritage = ParseClassHeritage();
  auto& class_body = ParseClassBody();
  return node_factory().NewClass(GetSourceCodeRange(), class_name, heritage,
                                 class_body);
}

const ast::Node& Parser::ParseClassBody() {
  NodeRangeScope scope(this);
  if (!CanPeekToken() || PeekToken() != ast::TokenKind::LeftBrace)
    return NewInvalidExpression(ErrorCode::ERROR_CLASS_EXPECT_LBRACE);
  return ParsePrimaryExpression();
}

const ast::Node& Parser::ParseClassHeritage() {
  if (!ConsumeTokenIf(ast::TokenKind::Extends))
    return NewElisionExpression();
  return ParseLeftHandSideExpression();
}

const ast::Node& Parser::ParseClassName() {
  if (!CanPeekToken() || PeekToken() != ast::SyntaxCode::Name)
    return NewEmptyName();
  if (PeekToken() == ast::TokenKind::Extends)
    return NewEmptyName();
  return ConsumeToken();
}

const ast::Node& Parser::ParseFunction(ast::FunctionKind kind) {
  const auto& name = CanPeekToken() && PeekToken() == ast::SyntaxCode::Name
                         ? ConsumeToken()
                         : NewEmptyName();
  const auto& parameter_list = ParseParameterList();
  auto& body = ParseFunctionBody();
  return node_factory().NewFunction(GetSourceCodeRange(), kind, name,
                                    parameter_list, body);
}

const ast::Node& Parser::ParseFunctionBody() {
  if (!CanPeekToken() || PeekToken() != ast::TokenKind::LeftBrace)
    return NewInvalidStatement(ErrorCode::ERROR_FUNCTION_EXPECT_LBRACE);
  NodeRangeScope scope(this);
  return ParseStatement();
}

const ast::Node& Parser::ParseMethod(ast::MethodKind method_kind,
                                     ast::FunctionKind kind) {
  auto& method_name = ParsePropertyName();
  const auto& parameter_list = ParseParameterList();
  auto& method_body = ParseFunctionBody();
  return node_factory().NewMethod(GetSourceCodeRange(), method_kind, kind,
                                  method_name, parameter_list, method_body);
}

const ast::Node& Parser::ParseParameterList() {
  NodeRangeScope scope(this);
  if (!ConsumeTokenIf(ast::TokenKind::LeftParenthesis)) {
    AddError(ErrorCode::ERROR_FUNCTION_EXPECT_LPAREN);
    return node_factory().NewParameterList(GetSourceCodeRange(), {});
  }
  if (ConsumeTokenIf(ast::TokenKind::RightParenthesis))
    return node_factory().NewParameterList(GetSourceCodeRange(), {});
  const auto& parameters = ParseBindingElements();
  ExpectPunctuator(ast::TokenKind::RightParenthesis,
                   ErrorCode::ERROR_FUNCTION_EXPECT_RPAREN);
  return node_factory().NewParameterList(GetSourceCodeRange(), parameters);
}

}  // namespace parser
}  // namespace joana

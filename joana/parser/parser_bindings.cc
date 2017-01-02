// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/ast/bindings.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/parser/parser.h"
#include "joana/parser/parser_error_codes.h"

namespace joana {
namespace parser {

namespace {

bool CanStartBindingElement(const ast::Node& token) {
  return token == ast::SyntaxCode::Name || token == ast::TokenKind::DotDotDot ||
         token == ast::TokenKind::LeftBrace ||
         token == ast::TokenKind::LeftBracket;
}

}  // namespace

// This function is called after consuming keyword or right parenthesis.
std::vector<const ast::Node*> Parser::ParseBindingElements() {
  std::vector<const ast::Node*> elements;
  while (CanPeekToken()) {
    const auto& element = ParseBindingElement();
    elements.push_back(&element);
    if (ConsumeTokenIf(ast::TokenKind::Comma))
      continue;
    if (!CanPeekToken() || !CanStartBindingElement(PeekToken()))
      break;
    if (is_separated_by_newline_ && !options_.disable_automatic_semicolon())
      break;
    AddError(element.range(), ErrorCode::ERROR_BINDING_EXPECT_COMMA);
    continue;
  }
  return std::move(elements);
}

const ast::Node& Parser::ParseBindingElement() {
  NodeRangeScope scope(this);
  if (PeekToken() == ast::SyntaxCode::Name)
    return ParseNameBindingElement();

  if (ConsumeTokenIf(ast::TokenKind::DotDotDot)) {
    const auto& element = ParseBindingElement();
    return node_factory().NewBindingRestElement(GetSourceCodeRange(), element);
  }

  if (ConsumeTokenIf(ast::TokenKind::LeftBrace))
    return ParseObjectBindingPattern();

  if (ConsumeTokenIf(ast::TokenKind::LeftBracket))
    return ParseArrayBindingPattern();

  AddError(ErrorCode::ERROR_BINDING_INVALID_ELEMENT);
  return node_factory().NewBindingInvalidElement(PeekToken().range());
}

// Factory helper functions
const ast::Node& Parser::NewArrayBindingPattern(
    const std::vector<const ast::Node*>& elements,
    const ast::Node& initializer) {
  return node_factory().NewArrayBindingPattern(GetSourceCodeRange(), elements,
                                               initializer);
}

const ast::Node& Parser::NewBindingCommaElement(const ast::Node& token) {
  DCHECK_EQ(token, ast::TokenKind::Comma);
  return node_factory().NewBindingCommaElement(token.range());
}

const ast::Node& Parser::NewBindingNameElement(const ast::Node& name,
                                               const ast::Node& initializer) {
  return node_factory().NewBindingNameElement(GetSourceCodeRange(), name,
                                              initializer);
}

const ast::Node& Parser::NewObjectBindingPattern(
    const std::vector<const ast::Node*>& elements,
    const ast::Node& initializer) {
  return node_factory().NewObjectBindingPattern(GetSourceCodeRange(), elements,
                                                initializer);
}

// Paring helper functions
const ast::Node& Parser::ParseArrayBindingPattern() {
  std::vector<const ast::Node*> elements;
  while (CanPeekToken() && PeekToken() != ast::TokenKind::RightBracket) {
    if (!elements.empty() &&
        *elements.back() == ast::SyntaxCode::BindingRestElement) {
      AddError(elements.back()->range(),
               ErrorCode::ERROR_BINDING_UNEXPECT_REST);
    }

    if (PeekToken() == ast::TokenKind::Comma) {
      elements.push_back(&NewBindingCommaElement(ConsumeToken()));
      continue;
    }

    if (!CanStartBindingElement(PeekToken())) {
      const auto& token = PeekToken();
      if (!SkipToListElement())
        break;
      AddError(token, ErrorCode::ERROR_BINDING_INVALID_ELEMENT);
      continue;
    }

    elements.push_back(&ParseBindingElement());
  }
  ConsumeTokenIf(ast::TokenKind::RightBracket);
  if (!ConsumeTokenIf(ast::TokenKind::Equal)) {
    return NewArrayBindingPattern(elements, NewElisionExpression());
  }
  const auto& initializer = ParseAssignmentExpression();
  return NewArrayBindingPattern(elements, initializer);
}

const ast::Node& Parser::ParseNameBindingElement() {
  const auto& name = ConsumeToken();
  if (!ConsumeTokenIf(ast::TokenKind::Equal))
    return NewBindingNameElement(name, NewElisionExpression());
  const auto& initializer = ParseAssignmentExpression();
  return NewBindingNameElement(name, initializer);
}

// Note: We don't report an error for rest binding element in object pattern to
// support [1].
// [1] https://github.com/sebmarkbage/ecmascript-rest-spread
const ast::Node& Parser::ParseObjectBindingPattern() {
  std::vector<const ast::Node*> elements;

  while (CanPeekToken() && PeekToken() != ast::TokenKind::RightBrace) {
    if (PeekToken() == ast::TokenKind::Comma) {
      AddError(ErrorCode::ERROR_BINDING_UNEXPECT_COMMA);
      elements.push_back(&NewBindingCommaElement(ConsumeToken()));
      continue;
    }

    if (!CanStartBindingElement(PeekToken())) {
      const auto& token = PeekToken();
      if (!SkipToListElement())
        break;
      AddError(token, ErrorCode::ERROR_BINDING_INVALID_ELEMENT);
      continue;
    }

    if (PeekToken() == ast::SyntaxCode::Name) {
      NodeRangeScope scope(this);
      const auto& name = ConsumeToken();
      if (!CanPeekToken()) {
        elements.push_back(
            &NewBindingNameElement(name, NewElisionExpression()));
        break;
      }
      if (ConsumeTokenIf(ast::TokenKind::Colon)) {
        const auto& element = ParseBindingElement();
        elements.push_back(&node_factory().NewBindingProperty(
            GetSourceCodeRange(), name, element));
      } else if (ConsumeTokenIf(ast::TokenKind::Equal)) {
        const auto& initializer = ParseAssignmentExpression();
        elements.push_back(&NewBindingNameElement(name, initializer));
      } else {
        elements.push_back(&node_factory().NewBindingNameElement(
            source_code().Slice(name.range().start(),
                                PeekToken().range().start()),
            name, NewElisionExpression()));
      }
    } else {
      elements.push_back(&ParseBindingElement());
    }

    if (!CanPeekToken())
      break;
    if (PeekToken() == ast::TokenKind::Comma) {
      elements.push_back(&NewBindingCommaElement(ConsumeToken()));
      continue;
    }

    if (!CanStartBindingElement(PeekToken()))
      continue;
    AddError(elements.back()->range(), ErrorCode::ERROR_BINDING_EXPECT_COMMA);
  }
  ConsumeTokenIf(ast::TokenKind::RightBrace);
  if (!ConsumeTokenIf(ast::TokenKind::Equal))
    return NewObjectBindingPattern(elements, NewElisionExpression());
  const auto& initializer = ParseAssignmentExpression();
  return NewObjectBindingPattern(elements, initializer);
}

}  // namespace parser
}  // namespace joana

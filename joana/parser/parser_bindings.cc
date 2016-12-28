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

bool CanStartBindingElement(const ast::Token& token) {
  return token.Is<ast::Name>() || token == ast::PunctuatorKind::DotDotDot ||
         token == ast::PunctuatorKind::LeftBrace ||
         token == ast::PunctuatorKind::LeftBracket;
}

}  // namespace

// This function is called after consuming keyword or right parenthesis.
std::vector<const ast::BindingElement*> Parser::ParseBindingElements() {
  std::vector<const ast::BindingElement*> elements;
  while (CanPeekToken()) {
    elements.push_back(&ParseBindingElement());
    if (!ConsumeTokenIf(ast::PunctuatorKind::Comma))
      break;
  }
  return std::move(elements);
}

const ast::BindingElement& Parser::ParseBindingElement() {
  SourceCodeRangeScope scope(this);
  if (PeekToken().Is<ast::Name>())
    return ParseNameBindingElement();

  if (ConsumeTokenIf(ast::PunctuatorKind::DotDotDot)) {
    const auto& element = ParseBindingElement();
    return node_factory().NewBindingRestElement(GetSourceCodeRange(), element);
  }

  if (ConsumeTokenIf(ast::PunctuatorKind::LeftBrace))
    return ParseObjectBindingPattern();

  if (ConsumeTokenIf(ast::PunctuatorKind::LeftBracket))
    return ParseArrayBindingPattern();

  AddError(ErrorCode::ERROR_BINDING_INVALID_ELEMENT);
  return node_factory().NewBindingInvalidElement(GetSourceCodeRange());
}

// Factory helper functions
const ast::BindingElement& Parser::NewArrayBindingPattern(
    const std::vector<const ast::BindingElement*>& elements,
    const ast::Expression& initializer) {
  return node_factory().NewArrayBindingPattern(GetSourceCodeRange(), elements,
                                               initializer);
}

const ast::BindingElement& Parser::NewBindingCommaElement(
    const ast::Token& token) {
  DCHECK_EQ(token, ast::PunctuatorKind::Comma);
  return node_factory().NewBindingCommaElement(token.range());
}

const ast::BindingElement& Parser::NewBindingNameElement(
    const ast::Name& name,
    const ast::Expression& initializer) {
  return node_factory().NewBindingNameElement(GetSourceCodeRange(), name,
                                              initializer);
}

const ast::BindingElement& Parser::NewObjectBindingPattern(
    const std::vector<const ast::BindingElement*>& elements,
    const ast::Expression& initializer) {
  return node_factory().NewObjectBindingPattern(GetSourceCodeRange(), elements,
                                                initializer);
}

// Paring helper functions
const ast::BindingElement& Parser::ParseArrayBindingPattern() {
  std::vector<const ast::BindingElement*> elements;
  while (CanPeekToken() && PeekToken() != ast::PunctuatorKind::RightBracket) {
    if (!elements.empty() && elements.back()->Is<ast::BindingRestElement>()) {
      AddError(elements.back()->range(),
               ErrorCode::ERROR_BINDING_UNEXPECT_REST);
    }

    if (PeekToken() == ast::PunctuatorKind::Comma) {
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
  ConsumeTokenIf(ast::PunctuatorKind::RightBracket);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Equal)) {
    return NewArrayBindingPattern(elements, NewElisionExpression());
  }
  const auto& initializer = ParseAssignmentExpression();
  return NewArrayBindingPattern(elements, initializer);
}

const ast::BindingElement& Parser::ParseNameBindingElement() {
  const auto& name = ConsumeToken().As<ast::Name>();
  if (!ConsumeTokenIf(ast::PunctuatorKind::Equal))
    return NewBindingNameElement(name, NewElisionExpression());
  const auto& initializer = ParseAssignmentExpression();
  return NewBindingNameElement(name, initializer);
}

// Note: We don't report an error for rest binding element in object pattern to
// support [1].
// [1] https://github.com/sebmarkbage/ecmascript-rest-spread
const ast::BindingElement& Parser::ParseObjectBindingPattern() {
  std::vector<const ast::BindingElement*> elements;

  while (CanPeekToken() && PeekToken() != ast::PunctuatorKind::RightBrace) {
    if (PeekToken() == ast::PunctuatorKind::Comma) {
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

    if (PeekToken().Is<ast::Name>()) {
      SourceCodeRangeScope scope(this);
      const auto& name = ConsumeToken().As<ast::Name>();
      if (!CanPeekToken()) {
        elements.push_back(
            &NewBindingNameElement(name, NewElisionExpression()));
        break;
      }
      if (ConsumeTokenIf(ast::PunctuatorKind::Colon)) {
        const auto& element = ParseBindingElement();
        elements.push_back(&node_factory().NewBindingProperty(
            GetSourceCodeRange(), name, element));
      } else if (ConsumeTokenIf(ast::PunctuatorKind::Equal)) {
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
    if (PeekToken() == ast::PunctuatorKind::Comma) {
      elements.push_back(&NewBindingCommaElement(ConsumeToken()));
      continue;
    }

    if (!CanStartBindingElement(PeekToken()))
      continue;
    AddError(elements.back()->range(), ErrorCode::ERROR_BINDING_EXPECT_COMMA);
  }
  ConsumeTokenIf(ast::PunctuatorKind::RightBrace);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Equal))
    return NewObjectBindingPattern(elements, NewElisionExpression());
  const auto& initializer = ParseAssignmentExpression();
  return NewObjectBindingPattern(elements, initializer);
}

}  // namespace parser
}  // namespace joana

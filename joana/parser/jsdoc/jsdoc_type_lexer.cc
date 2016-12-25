// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/jsdoc/jsdoc_type_lexer.h"

#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/jsdoc/jsdoc_error_codes.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/parser/utils/lexer_utils.h"

namespace joana {
namespace parser {

namespace {

bool IsTypeNamePart(base::char16 char_code) {
  return IsIdentifierPart(char_code) || char_code == '.';
}

}  // namespace

//
// JsDocTypeLexer
//
JsDocTypeLexer::JsDocTypeLexer(ParserContext* context,
                               CharacterReader* reader,
                               const ParserOptions& options)
    : context_(*context), options_(options), reader_(*reader) {
  current_token_ = NextToken();
}

JsDocTypeLexer::~JsDocTypeLexer() = default;

int JsDocTypeLexer::location() const {
  return reader_.location();
}

ast::NodeFactory& JsDocTypeLexer::node_factory() {
  return context_.node_factory();
}

const SourceCode& JsDocTypeLexer::source_code() const {
  return reader_.source_code();
}

void JsDocTypeLexer::AddError(JsDocErrorCode error_code) {
  context_.error_sink().AddError(ComputeTokenRange(),
                                 static_cast<int>(error_code));
}

bool JsDocTypeLexer::CanPeekChar() const {
  return reader_.CanPeekChar();
}

SourceCodeRange JsDocTypeLexer::ComputeTokenRange() const {
  return source_code().Slice(token_start_, location());
}

base::char16 JsDocTypeLexer::ConsumeChar() {
  return reader_.ConsumeChar();
}

bool JsDocTypeLexer::ConsumeCharIf(base::char16 char_code) {
  if (!CanPeekChar() || PeekChar() != char_code)
    return false;
  ConsumeChar();
  return true;
}

const ast::Token& JsDocTypeLexer::ConsumeToken() {
  auto& token = PeekToken();
  current_token_ = NextToken();
  return token;
}

base::char16 JsDocTypeLexer::PeekChar() const {
  return reader_.PeekChar();
}

const ast::Token& JsDocTypeLexer::PeekToken() const {
  DCHECK(current_token_);
  return *current_token_;
}

const ast::Token& JsDocTypeLexer::NewPunctuator(ast::PunctuatorKind kind) {
  return node_factory().NewPunctuator(ComputeTokenRange(), kind);
}

const ast::Token* JsDocTypeLexer::NextToken() {
  while (CanPeekChar() && IsWhitespace(PeekChar()))
    ConsumeChar();
  if (!CanPeekChar())
    return nullptr;
  token_start_ = location();
  switch (PeekChar()) {
    case '!':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::LogicalNot);
    case '(':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::LeftParenthesis);
    case ')':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::RightParenthesis);
    case ',':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::Comma);
    case '*':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::Times);
    case ':':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::Colon);
    case '<':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::LessThan);
    case '=':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::Equal);
    case '>':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::GreaterThan);
    case '?':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::Question);
    case '[':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::LeftBracket);
    case ']':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::RightBracket);
    case '{':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::LeftBrace);
    case '|':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::BitOr);
    case '}':
      ConsumeChar();
      return &NewPunctuator(ast::PunctuatorKind::RightBrace);
    case '.': {
      auto number_of_dots = 0;
      while (ConsumeCharIf('.'))
        ++number_of_dots;
      if (number_of_dots != 3) {
        AddError(JsDocErrorCode::ERROR_JSDOC_UNEXPECT_DOT);
        return &NewPunctuator(ast::PunctuatorKind::Invalid);
      }
      return &NewPunctuator(ast::PunctuatorKind::DotDotDot);
    }
  }
  if (IsIdentifierStart(PeekChar())) {
    ConsumeChar();
    while (CanPeekChar() && IsTypeNamePart(PeekChar()))
      ConsumeChar();
    return &node_factory().NewName(ComputeTokenRange());
  }
  ConsumeChar();
  return &NewPunctuator(ast::PunctuatorKind::Invalid);
}

}  // namespace parser
}  // namespace joana

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>
#include <vector>

#include "joana/parser/jsdoc/jsdoc_parser.h"

#include "base/auto_reset.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/jsdoc_tags.h"
#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/jsdoc/jsdoc_error_codes.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/type/type_lexer.h"
#include "joana/parser/type/type_parser.h"
#include "joana/parser/utils/character_reader.h"
#include "joana/parser/utils/lexer_utils.h"

namespace joana {
namespace parser {

namespace {

enum class Syntax {
  Unknown,
#define V(name, ...) name,
  FOR_EACH_JSDOC_TAG_SYNTAX(V)
#undef V
};

Syntax TagSyntaxOf(const ast::Node& name) {
#define V(underscore, capital, syntax)                                 \
  if (name.name_id() == static_cast<int>(ast::NameId::JsDoc##capital)) \
    return Syntax::syntax;
  FOR_EACH_JSDOC_TAG_NAME(V)
#undef V
  return Syntax::Unknown;
}

}  // namespace

//
// JsDocParser::NodeRangeScope
//
class JsDocParser::NodeRangeScope final {
 public:
  explicit NodeRangeScope(JsDocParser* parser)
      : auto_reset_(&parser->node_start_, parser->reader_->location()) {}

  ~NodeRangeScope() = default;

 private:
  base::AutoReset<int> auto_reset_;

  DISALLOW_COPY_AND_ASSIGN(NodeRangeScope);
};

//
// JsDocParser
//
JsDocParser::JsDocParser(ParserContext* context,
                         const SourceCodeRange& range,
                         const ParserOptions& options)
    : context_(*context),
      node_start_(range.start()),
      options_(options),
      reader_(new CharacterReader(range)) {}

JsDocParser::~JsDocParser() = default;

// ErrorSink utility functions
void JsDocParser::AddError(const SourceCodeRange& range,
                           JsDocErrorCode error_code) {
  context_.error_sink().AddError(range, error_code);
}

void JsDocParser::AddError(int start, int end, JsDocErrorCode error_code) {
  AddError(source_code().Slice(start, end), error_code);
}

void JsDocParser::AddError(JsDocErrorCode error_code) {
  AddError(ComputeNodeRange(), error_code);
}

// CharacterReader utility functions
int JsDocParser::location() const {
  return reader_->location();
}

const SourceCode& JsDocParser::source_code() const {
  return reader_->source_code();
}

bool JsDocParser::CanPeekChar() const {
  return reader_->CanPeekChar();
}

base::char16 JsDocParser::ConsumeChar() {
  return reader_->ConsumeChar();
}

bool JsDocParser::ConsumeCharIf(base::char16 char_code) {
  return reader_->ConsumeCharIf(char_code);
}

base::char16 JsDocParser::PeekChar() const {
  return reader_->PeekChar();
}

// Factory utility functions
ast::NodeFactory& JsDocParser::node_factory() {
  return context_.node_factory();
}

SourceCodeRange JsDocParser::ComputeNodeRange() const {
  return source_code().Slice(node_start_, location());
}

const ast::Node& JsDocParser::NewDocument(
    const std::vector<const ast::Node*>& nodes) {
  return node_factory().NewJsDocDocument(ComputeNodeRange(), nodes);
}

const ast::Node& JsDocParser::NewName() {
  return node_factory().NewName(ComputeNodeRange());
}

const ast::Node& JsDocParser::NewTagName() {
  return node_factory().NewName(ComputeNodeRange());
}

const ast::Node& JsDocParser::NewTagWithVector(
    const ast::Node& name,
    const std::vector<const ast::Node*>& parameters) {
  return node_factory().NewJsDocTag(ComputeNodeRange(), name, parameters);
}

const ast::Node& JsDocParser::NewText(const SourceCodeRange& range) {
  return node_factory().NewJsDocText(range);
}

const ast::Node& JsDocParser::NewText(int start, int end) {
  return NewText(source_code().Slice(start, end));
}

const ast::Node& JsDocParser::NewText() {
  return NewText(ComputeNodeRange());
}

// Parsing functions
// The entry point of JsDoc parser.
const ast::Node* JsDocParser::Parse() {
  NodeRangeScope scope(this);
  SkipWhitespaces();
  std::vector<const ast::Node*> nodes;
  auto number_of_tags = 0;
  auto text_start = location();
  while (CanPeekChar()) {
    const auto text_end = SkipToBlockTag();
    if (text_end > text_start)
      nodes.push_back(&NewText(text_start, text_end));
    if (!CanPeekChar())
      break;
    ++number_of_tags;
    NodeRangeScope scope(this);
    nodes.push_back(&ParseTag(ParseTagName()));
    SkipWhitespaces();
    text_start = location();
  }
  if (number_of_tags == 0)
    return nullptr;
  return &NewDocument(nodes);
}

// Extract text without leading and trailing whitespace.
const ast::Node& JsDocParser::ParseDescription() {
  SkipWhitespaces();
  const auto text_start = reader_->location();
  auto text_end = SkipToBlockTag();
  return NewText(text_start, text_end);
}

// Returns list of comma separated words.
std::vector<const ast::Node*> JsDocParser::ParseNameList() {
  SkipWhitespaces();
  NodeRangeScope scope(this);
  if (!ConsumeCharIf(kLeftBrace)) {
    AddError(JsDocErrorCode::ERROR_JSDOC_EXPECT_LBRACE);
    return {&NewText()};
  }
  const auto& names = ParseNames();
  if (!ConsumeCharIf(kRightBrace))
    AddError(JsDocErrorCode::ERROR_JSDOC_EXPECT_RBRACE);
  return std::move(names);
}

const ast::Node& JsDocParser::ParseName() {
  SkipWhitespaces();
  NodeRangeScope scope(this);
  while (CanPeekChar() && IsIdentifierPart(PeekChar()))
    ConsumeChar();
  return NewName();
}

std::vector<const ast::Node*> JsDocParser::ParseNames() {
  std::vector<const ast::Node*> names;
  SkipWhitespaces();
  while (CanPeekChar()) {
    if (!IsIdentifierStart(PeekChar()))
      break;
    names.push_back(&ParseName());
    SkipWhitespaces();
    if (!ConsumeCharIf(','))
      break;
    SkipWhitespaces();
  }
  return std::move(names);
}

// Extract text until newline
const ast::Node& JsDocParser::ParseSingleLine() {
  SkipWhitespaces();
  const auto text_start = location();
  auto text_end = text_start;
  while (CanPeekChar()) {
    const auto char_code = ConsumeChar();
    if (IsLineTerminator(char_code))
      break;
    if (IsWhitespace(char_code))
      continue;
    text_end = location();
  }
  return NewText(text_start, text_end);
}

// Called after consuming '@'.
const ast::Node& JsDocParser::ParseTag(const ast::Node& tag_name) {
  switch (TagSyntaxOf(tag_name)) {
    case Syntax::Description:
      SkipWhitespaces();
      return NewTag(tag_name, ParseDescription());
    case Syntax::NameList:
      return NewTagWithVector(tag_name, ParseNameList());
    case Syntax::Names:
      return NewTagWithVector(tag_name, ParseNames());
    case Syntax::None:
      return NewTag(tag_name);
    case Syntax::OptionalType:
      SkipWhitespaces();
      if (CanPeekChar() && PeekChar() == kLeftBrace)
        return NewTag(tag_name, ParseType());
      return NewTag(tag_name);
    case Syntax::SingleLine:
      return NewTag(tag_name, ParseSingleLine());
    case Syntax::Type:
      return NewTag(tag_name, ParseType());
    case Syntax::TypeDescription: {
      auto& type = ParseType();
      auto& description = ParseDescription();
      return NewTag(tag_name, type, description);
    }
    case Syntax::TypeNameDescription: {
      auto& type = ParseType();
      auto& name = ParseName();
      auto& description = ParseDescription();
      return NewTag(tag_name, type, name, description);
    }
    case Syntax::Unknown:
      AddError(tag_name.range(), JsDocErrorCode::ERROR_JSDOC_UNKNOWN_TAG);
      return NewTag(tag_name);
  }
  NOTREACHED() << "We should handle " << tag_name;
  return NewTag(tag_name);
}

const ast::Node& JsDocParser::ParseTagName() {
  DCHECK_EQ(PeekChar(), '@');
  NodeRangeScope scope(this);
  ConsumeChar();
  while (CanPeekChar() && IsIdentifierPart(PeekChar()))
    ConsumeChar();
  return NewTagName();
}

const ast::Node& JsDocParser::ParseType() {
  SkipWhitespaces();
  NodeRangeScope scope(this);
  if (!ConsumeCharIf(kLeftBrace)) {
    AddError(JsDocErrorCode::ERROR_JSDOC_EXPECT_LBRACE);
    return NewText();
  }
  const auto type_start = reader_->location();
  auto number_of_braces = 0;
  while (CanPeekChar()) {
    if (ConsumeCharIf(kLeftBrace)) {
      ++number_of_braces;
      continue;
    }
    if (PeekChar() == kRightBrace) {
      if (number_of_braces == 0)
        break;
      --number_of_braces;
      ConsumeChar();
      continue;
    }
    ConsumeChar();
  }
  TypeParser parser(&context_,
                    source_code().Slice(type_start, reader_->location()),
                    options_, TypeLexerMode::JsDoc);
  auto& type = parser.Parse();
  ConsumeCharIf(kRightBrace);
  return type;
}

int JsDocParser::SkipToBlockTag() {
  SkipWhitespaces();
  enum class State {
    Brace,
    InlineTag,
    Normal,
  } state = State::Normal;
  auto text_end = reader_->location();
  auto inline_tag_start = 0;
  while (CanPeekChar()) {
    switch (state) {
      case State::Brace:
        if (ConsumeCharIf('@')) {
          state = State::InlineTag;
          continue;
        }
        if (ConsumeCharIf(kLeftBrace))
          continue;
        state = State::Normal;
        continue;
      case State::Normal:
        if (PeekChar() == '@')
          return text_end;
        if (!IsWhitespace(PeekChar()))
          text_end = reader_->location() + 1;
        if (ConsumeChar() == kLeftBrace) {
          inline_tag_start = reader_->location() - 1;
          state = State::Brace;
          continue;
        }
        continue;
      case State::InlineTag:
        if (ConsumeChar() != kRightBrace)
          continue;
        state = State::Normal;
        text_end = reader_->location();
        continue;
    }
    NOTREACHED() << "We have missing case for state="
                 << static_cast<int>(state);
  }
  if (state == State::InlineTag) {
    AddError(inline_tag_start, reader_->location(),
             JsDocErrorCode::ERROR_JSDOC_EXPECT_RBRACE);
  }
  return text_end;
}

void JsDocParser::SkipWhitespaces() {
  while (CanPeekChar() && IsWhitespace(PeekChar()))
    ConsumeChar();
}

}  // namespace parser
}  // namespace joana

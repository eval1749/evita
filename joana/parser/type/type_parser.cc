// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/parser/type/type_parser.h"

#include "joana/ast/declarations.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/type/type_error_codes.h"
#include "joana/parser/type/type_lexer.h"
#include "joana/parser/utils/bracket_tracker.h"

namespace joana {
namespace parser {

namespace {

bool CanStartType(const ast::Node& token) {
  return token == ast::SyntaxCode::Name ||
         token == ast::TokenKind::LeftParenthesis ||
         token == ast::TokenKind::LeftBracket ||
         token == ast::TokenKind::LeftBrace ||
         token == ast::TokenKind::Question;
  token == ast::TokenKind::Times;
}

std::unique_ptr<BracketTracker> NewBracketTracker(
    ErrorSink* error_sink,
    const SourceCodeRange& source_code_range) {
  const auto descriptions = std::vector<BracketTracker::Description>{
      {ast::TokenKind::LeftParenthesis,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RPAREN),
       ast::TokenKind::RightParenthesis,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RPAREN)},
      {ast::TokenKind::LeftBrace,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RBRACE),
       ast::TokenKind::RightBrace,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RBRACE)},
      {ast::TokenKind::LeftBracket,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RBRACKET),
       ast::TokenKind::RightBracket,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RBRACKET)},
      {ast::TokenKind::LessThan,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RANGLE),
       ast::TokenKind::GreaterThan,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RANGLE)},
  };

  return std::make_unique<BracketTracker>(error_sink, source_code_range,
                                          descriptions);
}

}  // namespace

//
// TypeParser::TypeNodeScope
//
class TypeParser::TypeNodeScope final {
 public:
  explicit TypeNodeScope(TypeParser* parser)
      : parser_(*parser), node_start_(parser->node_start_) {
    parser_.node_start_ = parser_.PeekToken().range().start();
  }

  ~TypeNodeScope() { parser_.node_start_ = node_start_; }

 private:
  TypeParser& parser_;
  const int node_start_;

  DISALLOW_COPY_AND_ASSIGN(TypeNodeScope);
};

//
// TypeParser
//
TypeParser::TypeParser(ParserContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options,
                       TypeLexerMode mode)
    : bracket_tracker_(NewBracketTracker(&context->error_sink(), range)),
      context_(*context),
      lexer_(new TypeLexer(context, range, options, mode)),
      node_start_(range.start()),
      options_(options) {}

TypeParser::TypeParser(ParserContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options)
    : TypeParser(context, range, options, TypeLexerMode::Normal) {}

TypeParser::~TypeParser() = default;

ast::NodeFactory& TypeParser::node_factory() {
  return context_.node_factory();
}

const SourceCode& TypeParser::source_code() const {
  return lexer_->source_code();
}

void TypeParser::AddError(int start, int end, TypeErrorCode error_code) {
  AddError(source_code().Slice(start, end), error_code);
}

void TypeParser::AddError(const SourceCodeRange& range,
                          TypeErrorCode error_code) {
  context_.error_sink().AddError(range, static_cast<int>(error_code));
}

void TypeParser::AddError(const ast::Node& token, TypeErrorCode error_code) {
  AddError(token.range(), error_code);
}

void TypeParser::AddError(TypeErrorCode error_code) {
  if (CanPeekToken())
    return AddError(PeekToken().range(), error_code);
  AddError(lexer_->location(), error_code);
}

bool TypeParser::CanPeekToken() const {
  return lexer_->CanPeekToken();
}

const ast::Node& TypeParser::ConsumeToken() {
  const auto& token = lexer_->ConsumeToken();
  bracket_tracker_->Feed(token);
  return token;
}

const ast::Node& TypeParser::PeekToken() const {
  return lexer_->PeekToken();
}

void TypeParser::SkipTokensTo(ast::TokenKind kind) {
  while (CanPeekToken() && !ConsumeTokenIf(kind))
    ConsumeToken();
}

// Factory members
SourceCodeRange TypeParser::ComputeNodeRange() const {
  if (!CanPeekToken())
    return source_code().Slice(node_start_, lexer_->range().end());
  return source_code().Slice(node_start_, PeekToken().range().end());
}

const ast::Node& TypeParser::NewAnyType() {
  return node_factory().NewAnyType(ComputeNodeRange());
}

const ast::Node& TypeParser::NewFunctionType(ast::FunctionTypeKind kind,
                                             const ast::Node& parameter_list,
                                             const ast::Node& return_type) {
  return node_factory().NewFunctionType(ComputeNodeRange(), kind,
                                        parameter_list, return_type);
}

const ast::Node& TypeParser::NewInvalidType() {
  return node_factory().NewInvalidType(ComputeNodeRange());
}

const ast::Node& TypeParser::NewMemberType(const ast::Node& member,
                                           const ast::Node& name) {
  DCHECK(member == ast::SyntaxCode::TypeName ||
         member == ast::SyntaxCode::MemberType)
      << member;
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return node_factory().NewMemberType(ComputeNodeRange(), member, name);
}

const ast::Node& TypeParser::NewNullableType(const ast::Node& type) {
  return node_factory().NewNullableType(ComputeNodeRange(), type);
}

const ast::Node& TypeParser::NewNonNullableType(const ast::Node& type) {
  return node_factory().NewNonNullableType(ComputeNodeRange(), type);
}

const ast::Node& TypeParser::NewOptionalType(const ast::Node& type) {
  return node_factory().NewOptionalType(ComputeNodeRange(), type);
}

const ast::Node& TypeParser::NewProperty(const ast::Node& name,
                                         const ast::Node& type) {
  return node_factory().NewProperty(name.range(), name, type);
}

const ast::Node& TypeParser::NewRecordType(
    const std::vector<const ast::Node*>& members) {
  return node_factory().NewRecordType(ComputeNodeRange(), members);
}

const ast::Node& TypeParser::NewRestType(const ast::Node& type) {
  return node_factory().NewRestType(ComputeNodeRange(), type);
}

const ast::Node& TypeParser::NewTupleType(
    const std::vector<const ast::Node*>& members) {
  return node_factory().NewTupleType(ComputeNodeRange(), members);
}

const ast::Node& TypeParser::NewTypeApplication(
    const ast::Node& name,
    const ast::Node& argument_list) {
  DCHECK_EQ(name, ast::SyntaxCode::TypeName);
  return node_factory().NewTypeApplication(ComputeNodeRange(), name,
                                           argument_list);
}

const ast::Node& TypeParser::NewTypeGroup(const ast::Node& type) {
  return node_factory().NewTypeGroup(ComputeNodeRange(), type);
}

const ast::Node& TypeParser::NewTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  return node_factory().NewTypeName(name);
}

const ast::Node& TypeParser::NewUnionType(
    const std::vector<const ast::Node*>& members) {
  DCHECK(!members.empty());
  if (members.size() == 1)
    return *members.front();
  return node_factory().NewUnionType(ComputeNodeRange(), members);
}

const ast::Node& TypeParser::NewVoidType(const SourceCodeRange& range) {
  return node_factory().NewVoidType(range);
}

// Type ::=
//      | '?'
//      | '*'
//      | NullableType
//      | NonNullableType
//      | UnionType
//      | TypeName
//      | FunctionType
//      | TypeApplication
//      | TypeGroup
//      | RecordType
//      | TupleType
// NullableType ::= '?' Type
// NonNullableType ::= '!' Type
// RecordType ::= '{' (Name ':' Type ','?)* '}'
// TupleType ::= '[' (Type ',')* ']'
// UnionTYpe ::= Type ('|' Type*)
const ast::Node& TypeParser::Parse() {
  const auto& type = ParseUnionType();
  bracket_tracker_->Finish();
  if (CanPeekToken())
    AddError(PeekToken(), TypeErrorCode::ERROR_TYPE_UNEXPECT_TOKEN);
  return type;
}

const ast::Node& TypeParser::ParseFunctionType(const ast::Node& name) {
  if (!CanPeekToken() || PeekToken() != ast::TokenKind::LeftParenthesis)
    return NewTypeName(name);
  const auto& result = ParseParameters();
  const auto& parameter_list = *result.first;
  const auto kind = result.second;
  if (ConsumeTokenIf(ast::TokenKind::Colon))
    return NewFunctionType(kind, parameter_list, ParseType());
  const auto& return_type = NewVoidType(lexer_->location());
  return NewFunctionType(kind, parameter_list, return_type);
}

const ast::Node& TypeParser::ParseNameAsType(const ast::Node& name) {
  const auto& type_name = ParseQualifiedName(name);
  if (!CanPeekToken() || PeekToken() != ast::TokenKind::LessThan)
    return type_name;
  // TypeApplication ::= TypeName '<' (Type',')* '>'
  const auto& arguments = ParseTypeArguments();
  return NewTypeApplication(type_name, arguments);
}

// RecordType ::= '{' (Name ':' Type ','?)* '}'
const ast::Node& TypeParser::ParseRecordType() {
  std::vector<const ast::Node*> members;
  while (CanPeekToken() && PeekToken() != ast::TokenKind::RightBrace) {
    if (PeekToken() != ast::SyntaxCode::Name)
      break;
    const auto& name = ConsumeToken();
    if (!ConsumeTokenIf(ast::TokenKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    if (!CanPeekToken())
      break;
    const auto& type = ParseType();
    members.push_back(&NewProperty(name, type));
    if (!ConsumeTokenIf(ast::TokenKind::Comma))
      break;
  }
  SkipTokensTo(ast::TokenKind::RightBrace);
  return NewRecordType(members);
}

std::pair<const ast::Node*, ast::FunctionTypeKind>
TypeParser::ParseParameters() {
  TypeNodeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::TokenKind::LeftParenthesis);
  ConsumeToken();
  std::vector<const ast::Node*> parameters;
  auto kind = ast::FunctionTypeKind::Normal;
  auto expect_type = false;
  if (ConsumeTokenIf(ast::TokenKind::New)) {
    kind = ast::FunctionTypeKind::New;
    if (!ConsumeTokenIf(ast::TokenKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    expect_type = true;
  } else if (ConsumeTokenIf(ast::TokenKind::This)) {
    kind = ast::FunctionTypeKind::This;
    if (!ConsumeTokenIf(ast::TokenKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    expect_type = true;
  }
  while (CanPeekToken() && PeekToken() != ast::TokenKind::RightParenthesis) {
    expect_type = false;
    if (!CanStartType(PeekToken()))
      break;
    parameters.push_back(&ParseType());
    if (!CanPeekToken())
      break;
    if (PeekToken() == ast::TokenKind::RightParenthesis)
      break;
    expect_type = true;
    if (ConsumeTokenIf(ast::TokenKind::Comma))
      continue;
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COMMA);
  }
  if (expect_type)
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
  SkipTokensTo(ast::TokenKind::RightParenthesis);
  return std::make_pair(
      &node_factory().NewTuple(ComputeNodeRange(), parameters), kind);
}

const ast::Node& TypeParser::ParseQualifiedName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto* member = &NewTypeName(name);
  while (ConsumeTokenIf(ast::TokenKind::Dot)) {
    if (!CanPeekToken() || PeekToken() != ast::SyntaxCode::Name) {
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
      break;
    }
    member = &NewMemberType(*member, ConsumeToken());
  }
  return *member;
}

// TupleType ::= '[' (Name ','?)* ']'
const ast::Node& TypeParser::ParseTupleType() {
  std::vector<const ast::Node*> members;
  while (CanPeekToken() && PeekToken() != ast::TokenKind::RightBracket) {
    members.push_back(&ParseType());
    if (!CanPeekToken())
      break;
    if (ConsumeTokenIf(ast::TokenKind::Comma))
      continue;
    if (CanStartType(PeekToken())) {
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COMMA);
      continue;
    }
    break;
  }
  if (members.empty())
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
  SkipTokensTo(ast::TokenKind::RightBracket);
  return NewTupleType(members);
}

const ast::Node& TypeParser::ParseType() {
  if (!CanPeekToken()) {
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
    return NewInvalidType();
  }
  TypeNodeScope scope(this);
  const auto& type = ParseTypeBeforeEqual();
  if (!ConsumeTokenIf(ast::TokenKind::Equal))
    return type;
  return NewOptionalType(type);
}

const ast::Node& TypeParser::ParseTypeArguments() {
  DCHECK_EQ(PeekToken(), ast::TokenKind::LessThan);
  TypeNodeScope scope(this);
  ConsumeToken();
  std::vector<const ast::Node*> arguments;
  while (CanPeekToken() && PeekToken() != ast::TokenKind::GreaterThan) {
    arguments.push_back(&ParseType());
    if (!ConsumeTokenIf(ast::TokenKind::Comma))
      break;
  }
  SkipTokensTo(ast::TokenKind::GreaterThan);
  return node_factory().NewTuple(ComputeNodeRange(), arguments);
}

const ast::Node& TypeParser::ParseTypeBeforeEqual() {
  TypeNodeScope scope(this);
  if (ConsumeTokenIf(ast::TokenKind::Times))
    return NewAnyType();
  if (PeekToken() == ast::TokenKind::Question) {
    const auto& question = ConsumeToken();
    if (CanPeekToken() && CanStartType(PeekToken()))
      return NewNullableType(ParseType());
    return node_factory().NewUnknownType(question.range());
  }
  if (ConsumeTokenIf(ast::TokenKind::LogicalNot))
    return NewNonNullableType(ParseType());
  if (PeekToken() == ast::TokenKind::Function)
    return ParseFunctionType(ConsumeToken());
  if (PeekToken() == ast::SyntaxCode::Name)
    return ParseNameAsType(ConsumeToken());
  if (ConsumeTokenIf(ast::TokenKind::LeftBrace))
    return ParseRecordType();
  if (ConsumeTokenIf(ast::TokenKind::LeftBracket))
    return ParseTupleType();
  if (ConsumeTokenIf(ast::TokenKind::LeftParenthesis))
    return ParseTypeGroup();
  if (ConsumeTokenIf(ast::TokenKind::DotDotDot))
    return NewRestType(ParseType());
  ConsumeToken();
  return NewInvalidType();
}

const ast::Node& TypeParser::ParseTypeGroup() {
  const auto& type = ParseUnionType();
  ConsumeTokenIf(ast::TokenKind::RightParenthesis);
  return NewTypeGroup(type);
}

const ast::Node& TypeParser::ParseUnionType() {
  if (!CanPeekToken()) {
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
    return NewInvalidType();
  }
  TypeNodeScope scope(this);
  std::vector<const ast::Node*> types;
  types.push_back(&ParseType());
  while (CanPeekToken() && ConsumeTokenIf(ast::TokenKind::BitOr))
    types.push_back(&ParseType());
  return NewUnionType(types);
}

}  // namespace parser
}  // namespace joana

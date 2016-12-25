// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_TYPE_TYPE_PARSER_H_
#define JOANA_PARSER_TYPE_TYPE_PARSER_H_

#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "base/macros.h"

namespace joana {
class ParserContext;
class ParserOptions;
class SourceCode;
class SourceCodeRange;

namespace ast {
class Name;
enum class NameId;
enum class FunctionTypeKind;
class NodeFactory;
class Punctuator;
enum class PunctuatorKind;
class Token;
class Type;
}

namespace parser {

enum class TypeErrorCode;
class TypeLexer;

//
// TypeParser parses text representation of type in specified range of
// source code.
//
class TypeParser final {
  using Name = ast::Name;
  using Token = ast::Token;
  using Type = ast::Type;
  using RecordMember = std::pair<const Name*, const Type*>;

 public:
  TypeParser(ParserContext* context,
             const SourceCodeRange& range,
             const ParserOptions& options);
  ~TypeParser();

  const Type& Parse();

 private:
  using BracketPair = std::pair<const ast::Punctuator*, TypeErrorCode>;
  class TypeNodeScope;

  ast::NodeFactory& node_factory();
  const SourceCode& source_code() const;

  void AddError(int start, int end, TypeErrorCode error_code);
  void AddError(const SourceCodeRange& range, TypeErrorCode error_code);
  void AddError(const Token& token, TypeErrorCode error_code);
  void AddError(TypeErrorCode error_code);

  // Lexer utility members
  bool CanPeekToken() const;
  void CheckCloseBracket(const ast::Token& bracket,
                         const ast::PunctuatorKind open_bracket,
                         TypeErrorCode error_code);
  const Token& ConsumeToken();

  template <typename T>
  bool ConsumeTokenIf(T expected) {
    if (!CanPeekToken() || PeekToken() != expected)
      return false;
    ConsumeToken();
    return true;
  }

  const Token& PeekToken() const;
  void SkipTokensTo(ast::PunctuatorKind kind);

  // Factory members
  SourceCodeRange ComputeNodeRange() const;
  const Type& NewAnyType();
  const Type& NewFunctionType(ast::FunctionTypeKind method_kind,
                              const std::vector<const Type*>& parameters,
                              const Type& return_type);
  const Type& NewInvalidType();
  const Type& NewNullableType(const Type& type);
  const Type& NewNonNullableType(const Type& type);
  const Type& NewOptionalType(const Type& type);
  const Type& NewRecordType(const std::vector<RecordMember>& members);
  const Type& NewRestType(const Type& type);
  const Type& NewTupleType(const std::vector<const Type*>& members);
  const Type& NewTypeApplication(const Name& name,
                                 const std::vector<const Type*>& types);
  const Type& NewTypeName(const Name& name);
  const Type& NewUnionType(const std::vector<const Type*>& members);
  const Type& NewUnknownType();
  const Type& NewVoidType(const SourceCodeRange& range);

  // Parser for each syntax
  const Type& ParseTypeApplication(const Name& name);
  const Type& ParseFunctionType(const Name& name);
  const Type& ParseNameAsType(const Name& name);
  const Type& ParseRecordType();
  const Type& ParseTupleType();
  const Type& ParseType();
  const Type& ParseTypeBeforeEqual();

  std::stack<BracketPair> brackets_;
  ParserContext& context_;
  const std::unique_ptr<TypeLexer> lexer_;
  const ParserOptions& options_;
  int node_start_ = -1;

  DISALLOW_COPY_AND_ASSIGN(TypeParser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_TYPE_TYPE_PARSER_H_

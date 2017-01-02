// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_TYPE_TYPE_PARSER_H_
#define JOANA_PARSER_TYPE_TYPE_PARSER_H_

#include <memory>
#include <utility>
#include <vector>

#include "base/macros.h"

namespace joana {
class ParserContext;
class ParserOptions;
class SourceCode;
class SourceCodeRange;

namespace ast {
enum class TokenKind;
enum class FunctionTypeKind;
class Node;
class NodeFactory;
enum class TokenKind;
}

namespace parser {

class BracketTracker;
enum class TypeErrorCode;
class TypeLexer;
enum class TypeLexerMode;

//
// TypeParser parses text representation of type in specified range of
// source code.
//
class TypeParser final {
 public:
  TypeParser(ParserContext* context,
             const SourceCodeRange& range,
             const ParserOptions& options,
             TypeLexerMode mode);
  TypeParser(ParserContext* context,
             const SourceCodeRange& range,
             const ParserOptions& options);
  ~TypeParser();

  const ast::Node& Parse();

 private:
  class TypeNodeScope;

  ast::NodeFactory& node_factory();
  const SourceCode& source_code() const;

  void AddError(int start, int end, TypeErrorCode error_code);
  void AddError(const SourceCodeRange& range, TypeErrorCode error_code);
  void AddError(const ast::Node& token, TypeErrorCode error_code);
  void AddError(TypeErrorCode error_code);

  // Lexer utility members
  bool CanPeekToken() const;
  const ast::Node& ConsumeToken();

  template <typename T>
  bool ConsumeTokenIf(T expected) {
    if (!CanPeekToken() || PeekToken() != expected)
      return false;
    ConsumeToken();
    return true;
  }

  const ast::Node& PeekToken() const;
  void SkipTokensTo(ast::TokenKind kind);

  // Factory members
  SourceCodeRange ComputeNodeRange() const;
  const ast::Node& NewAnyType();
  const ast::Node& NewFunctionType(ast::FunctionTypeKind method_kind,
                                   const ast::Node& parameter_list,
                                   const ast::Node& return_type);
  const ast::Node& NewInvalidType();
  const ast::Node& NewNullableType(const ast::Node& type);
  const ast::Node& NewNonNullableType(const ast::Node& type);
  const ast::Node& NewOptionalType(const ast::Node& type);
  const ast::Node& NewRecordType(const std::vector<const ast::Node*>& members);
  const ast::Node& NewRestType(const ast::Node& type);
  const ast::Node& NewTupleType(const std::vector<const ast::Node*>& members);
  const ast::Node& NewTypeApplication(const ast::Node& name,
                                      const ast::Node& argument_list);
  const ast::Node& NewTypeGroup(const ast::Node& type);
  const ast::Node& NewTypeName(const ast::Node& name);
  const ast::Node& NewTypeProperty(const ast::Node& name,
                                   const ast::Node& type);
  const ast::Node& NewUnionType(const std::vector<const ast::Node*>& members);
  const ast::Node& NewVoidType(const SourceCodeRange& range);

  // Parser for each syntax
  const ast::Node& ParseFunctionType(const ast::Node& name);
  const ast::Node& ParseNameAsType(const ast::Node& name);
  std::pair<const ast::Node*, ast::FunctionTypeKind> ParseParameters();
  const ast::Node& ParseRecordType();
  const ast::Node& ParseTupleType();
  const ast::Node& ParseType();
  const ast::Node& ParseTypeArguments();
  const ast::Node& ParseTypeBeforeEqual();
  const ast::Node& ParseTypeGroup();
  const ast::Node& ParseUnionType();

  const std::unique_ptr<BracketTracker> bracket_tracker_;
  ParserContext& context_;
  const std::unique_ptr<TypeLexer> lexer_;
  const ParserOptions& options_;
  int node_start_;

  DISALLOW_COPY_AND_ASSIGN(TypeParser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_TYPE_TYPE_PARSER_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_JSDOC_JSDOC_PARSER_H_
#define JOANA_PARSER_JSDOC_JSDOC_PARSER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace joana {
class ParserContext;
class ParserOptions;
class SourceCode;
class SourceCodeRange;

namespace ast {
class JsDocDocument;
class JsDocNode;
class Name;
class Node;
class NodeFactory;
class StringLiteral;
class Token;
class Type;
}

namespace parser {

class CharacterReader;
enum class JsDocErrorCode;

//
// JsDocParser
//
class JsDocParser final {
  using JsDocDocument = ast::JsDocDocument;
  using JsDocNode = ast::JsDocNode;
  using Name = ast::Name;
  using Node = ast::Node;
  using Token = ast::Token;

 public:
  JsDocParser(ParserContext* context,
              const SourceCodeRange& range,
              const ParserOptions& options);
  ~JsDocParser();

  const JsDocDocument* Parse();

 private:
  class NodeRangeScope;

  // ErrorSink utility functions
  void AddError(const SourceCodeRange& range, JsDocErrorCode error_code);
  void AddError(JsDocErrorCode error_code);

  // Factory functions
  ast::NodeFactory& node_factory();

  SourceCodeRange ComputeNodeRange() const;

  const JsDocDocument& NewDocument(const std::vector<const JsDocNode*>& nodes);
  const JsDocNode& NewName();
  const Name& NewTagName();
  const JsDocNode& NewTagWithVector(
      const Name& tag_name,
      const std::vector<const JsDocNode*>& parameters);
  const JsDocNode& NewText(const SourceCodeRange& range);
  const JsDocNode& NewText(int start, int end);
  const JsDocNode& NewText();
  const JsDocNode& NewType(const ast::Type& type);

  // |NewTag()| is a help function wrapping |NewTagWithVector()| to avoid
  // using |&| before members.
  template <typename... Parameters>
  const JsDocNode& NewTag(const Name& tag_name,
                          const Parameters&... parameters) {
    return NewTagWithVector(tag_name, {&parameters...});
  }

  // Reader utility functions
  int location() const;
  const SourceCode& source_code() const;

  bool CanPeekChar() const;
  base::char16 ConsumeChar();
  bool ConsumeCharIf(base::char16 char_code);
  base::char16 PeekChar() const;

  // Parsing functions
  const JsDocNode& ParseDescription();
  const JsDocNode& ParseName();
  std::vector<const JsDocNode*> ParseNameList();
  std::vector<const JsDocNode*> ParseNames();
  const JsDocNode& ParseTag(const Name& tag_name);
  const Name& ParseTagName();
  const JsDocNode& ParseType();

  void SkipWhitespaces();

  ParserContext& context_;
  int node_start_;
  const ParserOptions& options_;
  std::unique_ptr<CharacterReader> reader_;

  DISALLOW_COPY_AND_ASSIGN(JsDocParser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_JSDOC_JSDOC_PARSER_H_

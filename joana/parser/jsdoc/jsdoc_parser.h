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
class Node;
class NodeFactory;
}  // namespace ast

namespace parser {

class CharacterReader;
enum class JsDocErrorCode;

//
// JsDocParser
//
class JsDocParser final {
 public:
  // |range| contains "/**" and "*/".
  JsDocParser(ParserContext* context,
              const SourceCodeRange& range,
              const ParserOptions& options);
  ~JsDocParser();

  const ast::Node* Parse();

 private:
  class NodeRangeScope;

  // ErrorSink utility functions
  void AddError(const SourceCodeRange& range, JsDocErrorCode error_code);
  void AddError(int start, int end, JsDocErrorCode error_code);
  void AddError(JsDocErrorCode error_code);

  // Factory functions
  ast::NodeFactory& node_factory();

  SourceCodeRange ComputeNodeRange() const;

  const ast::Node& NewDocument(const std::vector<const ast::Node*>& nodes);
  const ast::Node& NewName();
  const ast::Node& NewTagName();
  const ast::Node& NewTagWithVector(
      const ast::Node& tag_name,
      const std::vector<const ast::Node*>& parameters);
  const ast::Node& NewText(const SourceCodeRange& range);
  const ast::Node& NewText(int start, int end);
  const ast::Node& NewText();

  // |NewTag()| is a help function wrapping |NewTagWithVector()| to avoid
  // using |&| before members.
  template <typename... Parameters>
  const ast::Node& NewTag(const ast::Node& tag_name,
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
  const ast::Node& ParseDescription();
  const ast::Node& ParseModifies();
  const ast::Node& ParseName();
  std::vector<const ast::Node*> ParseNameList();
  std::vector<const ast::Node*> ParseNames();
  const ast::Node& ParseSingleLine();
  const ast::Node& ParseTag(const ast::Node& tag_name);
  const ast::Node& ParseTagName();
  const ast::Node& ParseType();

  // Returns start of training whitespace or before block tag.
  int SkipToBlockTag();
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

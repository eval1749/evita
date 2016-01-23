// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_
#define EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

#include <stdint.h>

#include <stack>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace visuals {

class ContainerNode;
class Document;
class ElementNode;
class Node;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// NodeTreeBuilder
//
class NodeTreeBuilder final {
 public:
  explicit NodeTreeBuilder(ContainerNode* container);
  NodeTreeBuilder();
  ~NodeTreeBuilder();

  NodeTreeBuilder& AddText(base::StringPiece16 text);
  NodeTreeBuilder& AddShape(const std::vector<uint8_t>& data);
  NodeTreeBuilder& Begin(base::StringPiece16 tag_name);
  NodeTreeBuilder& Begin(base::StringPiece16 tag_name, base::StringPiece16 id);
  Document* Build();
  NodeTreeBuilder& ClassList(const std::vector<base::string16>& names);
  NodeTreeBuilder& End(base::StringPiece16 tag_name);
  void Finish(ContainerNode* node);
  NodeTreeBuilder& SetInlineStyle(const css::Style& style);

 private:
  NodeTreeBuilder& AddInternal(Node* node);
  NodeTreeBuilder& BeginInternal(ElementNode* node);

  std::stack<Node*> nodes_;
  Document* new_document_;
  Document* const document_;

  DISALLOW_COPY_AND_ASSIGN(NodeTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

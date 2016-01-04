// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_
#define EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

#include <stack>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace visuals {

class ContainerNode;
class Document;
class Element;
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

  NodeTreeBuilder& AddText(const base::StringPiece16& text);
  NodeTreeBuilder& Begin(const base::StringPiece16& tag_name);
  NodeTreeBuilder& Begin(const base::StringPiece16& tag_name,
                         const base::StringPiece16& id);
  Document* Build();
  NodeTreeBuilder& ClassList(const std::vector<base::string16>& names);
  NodeTreeBuilder& End(const base::StringPiece16& tag_name);
  void Finish(ContainerNode* node);

  // Node
  NodeTreeBuilder& SetStyle(const css::Style& style);

 private:
  NodeTreeBuilder& AddInternal(Node* node);
  NodeTreeBuilder& BeginInternal(Element* node);

  std::stack<Node*> nodes_;
  Document* new_document_;
  Document* const document_;

  DISALLOW_COPY_AND_ASSIGN(NodeTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_
#define EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

#include <memory>
#include <stack>

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
  std::unique_ptr<Document> Build();
  NodeTreeBuilder& End(const base::StringPiece16& tag_name);
  void Finish(ContainerNode* node);

  // Node
  NodeTreeBuilder& SetStyle(const css::Style& style);

 private:
  NodeTreeBuilder& AddInternal(std::unique_ptr<Node> node);
  NodeTreeBuilder& BeginInternal(std::unique_ptr<Element> node);

  std::stack<Node*> nodes_;
  std::unique_ptr<Document> new_document_;
  Document* const document_;

  DISALLOW_COPY_AND_ASSIGN(NodeTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_TREE_BUILDER_H_

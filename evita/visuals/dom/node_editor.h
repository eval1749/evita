// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_EDITOR_H_
#define EVITA_VISUALS_DOM_NODE_EDITOR_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class ContainerNode;
class Document;
class FloatColor;
class FloatRect;
class FloatSize;
class Node;
class TextNode;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// NodeEditor
//
class NodeEditor final {
 public:
  NodeEditor();
  ~NodeEditor();

  // Node
  void DidLayout(Node* node);
  void DidMove(Node* node);
  void DidPaint(Node* node);
  void SetBounds(Node* node, const FloatRect& new_bounds);
  void SetLayoutClean(Node* node);

  // ContainerNode
  template <typename T>
  T* AppendChild(ContainerNode* container, std::unique_ptr<T>&& child) {
    static_assert(std::is_base_of<Node, T>::value, "Node should be base of T");
    const auto& result = child.get();
    AppendChild(container, std::unique_ptr<Node>(child.release()));
    return result;
  }

  Node* AppendChild(ContainerNode* container, std::unique_ptr<Node> child);
  std::unique_ptr<Node> RemoveChild(ContainerNode* container, Node* child);
  void SetStyle(Node* node, const css::Style& style);

  void SetContentChanged(Node* node);
  void SetShouldPaint(Node* node);
  void WillDestroy(Node* node);

  // TextNode
  void SetBaseline(TextNode* node, float new_baseline);

  // Document
  void SetViewportSize(Document* document, const FloatSize& size);

 private:
  void DidChangeChild(ContainerNode* container);
  void ScheduleVisualUpdateIfNeeded(Node* node);

  DISALLOW_COPY_AND_ASSIGN(NodeEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_EDITOR_H_

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_EDITOR_H_
#define EVITA_VISUALS_DOM_NODE_EDITOR_H_

#include "base/macros.h"

namespace visuals {

class ContainerNode;
class Document;
class Element;
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

  void AppendChild(ContainerNode* container, Node* new_child);
  void InsertBefore(ContainerNode* container, Node* new_child, Node* ref_child);
  void RemoveChild(ContainerNode* container, Node* old_child);
  void SetStyle(Element* element, const css::Style& style);

  void WillDestroy(Node* node);

 private:
  void DidChangeChild(ContainerNode* container);
  void RegisterElementIdForSubtree(const Node& node);
  void ScheduleVisualUpdateIfNeeded(Node* node);
  void UnregisterElementIdForSubtree(const Node& node);

  DISALLOW_COPY_AND_ASSIGN(NodeEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_EDITOR_H_

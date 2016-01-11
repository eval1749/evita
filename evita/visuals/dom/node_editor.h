// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_EDITOR_H_
#define EVITA_VISUALS_DOM_NODE_EDITOR_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace visuals {

class ContainerNode;
class Document;
class Element;
class Node;
class Text;

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
  void AddClass(Element* element, const base::StringPiece16& class_name);
  void InsertBefore(ContainerNode* container, Node* new_child, Node* ref_child);
  void RemoveChild(ContainerNode* container, Node* old_child);
  void RemoveClass(Element* element, const base::StringPiece16& class_name);
  void ReplaceChild(ContainerNode* container, Node* new_child, Node* old_child);
  void SetInlineStyle(Element* element, const css::Style& style);

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

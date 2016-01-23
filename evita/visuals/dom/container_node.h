// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_CONTAINER_NODE_H_
#define EVITA_VISUALS_DOM_CONTAINER_NODE_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "evita/visuals/dom/node.h"
#include "evita/visuals/dom/children.h"

namespace visuals {

class NodeEditor;

//////////////////////////////////////////////////////////////////////
//
// ContainerNode
//
class ContainerNode : public Node {
  DECLARE_VISUAL_NODE_ABSTRACT_CLASS(ContainerNode, Node);

 public:
  ~ContainerNode() override;

  Children child_nodes() const { return Children(*this); }
  Node* first_child() const { return first_child_; }
  Node* last_child() const { return last_child_; }

  bool IsChildrenChanged() const { return is_children_changed_; }
  bool IsSubtreeChanged() const { return is_subtree_changed_; }

 protected:
  ContainerNode(Document* document,
                base::StringPiece16 tag_name,
                base::StringPiece16 id);
  explicit ContainerNode(Document* document, base::StringPiece16 tag_name);

 private:
  friend class NodeEditor;

  // gc::Visitable
  void Accept(gc::Visitor* visitor) override;

  // For ease of using list of child nodes, we don't use |Node*|
  Node* first_child_ = nullptr;
  Node* last_child_ = nullptr;

  // |is_children_changed_| is true if one of child is changed affects
  // siblings or this container node. This flag is also true adding/removing
  // child.
  bool is_children_changed_ = false;

  // |is_subtree_changed_| is true if |is_changed_changed_| is true for one of
  // descendants, otherwise false.
  bool is_subtree_changed_ = false;

  DISALLOW_COPY_AND_ASSIGN(ContainerNode);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_CONTAINER_NODE_H_

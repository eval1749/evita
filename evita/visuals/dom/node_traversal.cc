// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_traversal.h"

#include "evita/visuals/dom/ancestors.h"
#include "evita/visuals/dom/container_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NodeTraversal
//

Node* NodeTraversal::FirstChildOf(const Node& node) {
  const auto container = node.as<ContainerNode>();
  if (!container)
    return nullptr;
  return container->first_child();
}

Node* NodeTraversal::LastChildOf(const Node& node) {
  const auto container = node.as<ContainerNode>();
  if (!container)
    return nullptr;
  return container->last_child();
}

Node* NodeTraversal::NextOf(const Node& node) {
  if (const auto first_child = FirstChildOf(node))
    return first_child;
  if (const auto next_sibling = NextSiblingOf(node))
    return next_sibling;
  for (const auto& runner : Node::Ancestors(node)) {
    if (const auto next = NextSiblingOf(*runner))
      return next;
  }
  return nullptr;
}

Node* NodeTraversal::NextSiblingOf(const Node& node) {
  return node.next_sibling();
}

ContainerNode* NodeTraversal::ParentOf(const Node& node) {
  return node.parent();
}

Node* NodeTraversal::PreviousSiblingOf(const Node& node) {
  return node.previous_sibling();
}

}  // namespace visuals

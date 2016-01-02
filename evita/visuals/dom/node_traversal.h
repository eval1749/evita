// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_TRAVERSAL_H_
#define EVITA_VISUALS_DOM_NODE_TRAVERSAL_H_

#include "base/macros.h"

namespace visuals {

class Node;
class ContainerNode;

//////////////////////////////////////////////////////////////////////
//
// NodeTraversal
//
class NodeTraversal {
 public:
  static Node* FirstChildOf(const Node& node);
  static Node* LastChildOf(const Node& node);
  static Node* NextOf(const Node& node);
  static Node* NextSiblingOf(const Node& node);
  static ContainerNode* ParentOf(const Node& node);
  static Node* PreviousSiblingOf(const Node& node);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(NodeTraversal);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_TRAVERSAL_H_

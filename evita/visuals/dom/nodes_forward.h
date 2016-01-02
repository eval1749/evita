// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODES_FORWARD_H_
#define EVITA_VISUALS_DOM_NODES_FORWARD_H_

namespace visuals {

#define FOR_EACH_VISUAL_BOX(V) \
  V(Document)                  \
  V(Element)                   \
  V(TextNode)

#define FOR_EACH_ABSTRACT_VISUAL_BOX(V) \
  V(Node)                               \
  V(ContainerNode)

// Forward declarations
class Node;
#define V(name) class name;
FOR_EACH_VISUAL_BOX(V)
FOR_EACH_ABSTRACT_VISUAL_BOX(V)
#undef V
class NodeVisitor;

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODES_FORWARD_H_

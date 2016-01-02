// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_VISITOR_H_
#define EVITA_VISUALS_DOM_NODE_VISITOR_H_

#include "base/macros.h"
#include "evita/visuals/dom/nodes_forward.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NodeVisitor
//
class NodeVisitor {
 public:
  void Visit(const Node& node);
  void Visit(Node* node);

#define V(name) virtual void Visit##name(name* node) = 0;
  FOR_EACH_VISUAL_NODE(V)
#undef V
 protected:
  NodeVisitor();
  virtual ~NodeVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeVisitor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_VISITOR_H_

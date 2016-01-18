// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_visitor.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/shape.h"
#include "evita/visuals/dom/text.h"

namespace visuals {

NodeVisitor::NodeVisitor() {}
NodeVisitor::~NodeVisitor() {}

void NodeVisitor::Visit(const Node& node) {
  Visit(&const_cast<Node&>(node));
}

void NodeVisitor::Visit(Node* node) {
  node->Accept(this);
}

#define V(name) \
  void name::Accept(NodeVisitor* visitor) { visitor->Visit##name(this); }
FOR_EACH_VISUAL_NODE(V)
#undef V

}  // namespace visuals

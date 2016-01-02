// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/container_node.h"

#include "base/logging.h"
#include "evita/visuals/dom/node_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerNode
//

ContainerNode::ContainerNode(Document* document,
                             const base::StringPiece16& tag_name,
                             const base::StringPiece16& id)
    : Node(document, tag_name, id) {}

ContainerNode::ContainerNode(Document* document,
                             const base::StringPiece16& tag_name)
    : Node(document, tag_name) {}

ContainerNode::~ContainerNode() {
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), parent());
  auto runner = first_child_;
  while (runner) {
    const auto next_child = runner->next_sibling();
    NodeEditor().WillDestroy(runner);
    delete runner;
    runner = next_child;
  }
}

}  // namespace visuals

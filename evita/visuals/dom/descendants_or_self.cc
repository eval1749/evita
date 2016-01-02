// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/descendants_or_self.h"

#include "base/logging.h"
#include "evita/visuals/dom/container_node.h"
#include "evita/visuals/dom/node_traversal.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::DescendantsOrSelf
//
Node::DescendantsOrSelf::DescendantsOrSelf(const Node& node) : node_(&node) {}
Node::DescendantsOrSelf::~DescendantsOrSelf() {}

Node::DescendantsOrSelf::Iterator Node::DescendantsOrSelf::begin() const {
  return Iterator(const_cast<Node*>(node_));
}

Node::DescendantsOrSelf::Iterator Node::DescendantsOrSelf::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Node::DescendantsOrSelf::Iterator
//
Node::DescendantsOrSelf::Iterator::Iterator(Node* node) : node_(node) {}

Node::DescendantsOrSelf::Iterator::Iterator(const Iterator& other)
    : node_(other.node_) {}

Node::DescendantsOrSelf::Iterator::~Iterator() {}

Node* Node::DescendantsOrSelf::Iterator::operator*() const {
  DCHECK(node_);
  return node_;
}

Node* Node::DescendantsOrSelf::Iterator::operator->() const {
  DCHECK(node_);
  return node_;
}

Node::DescendantsOrSelf::Iterator& Node::DescendantsOrSelf::Iterator::
operator++() {
  DCHECK(node_);
  node_ = NodeTraversal::NextOf(*node_);
  return *this;
}

bool Node::DescendantsOrSelf::Iterator::operator==(
    const Iterator& other) const {
  return node_ == other.node_;
}

bool Node::DescendantsOrSelf::Iterator::operator!=(
    const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals

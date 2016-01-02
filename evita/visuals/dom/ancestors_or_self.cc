// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/ancestors_or_self.h"

#include "base/logging.h"
#include "evita/visuals/dom/container_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::AncestorsOrSelf
//
Node::AncestorsOrSelf::AncestorsOrSelf(const Node& node) : node_(&node) {}
Node::AncestorsOrSelf::~AncestorsOrSelf() {}

Node::AncestorsOrSelf::Iterator Node::AncestorsOrSelf::begin() const {
  return Iterator(const_cast<Node*>(node_));
}

Node::AncestorsOrSelf::Iterator Node::AncestorsOrSelf::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Node::AncestorsOrSelf::Iterator
//
Node::AncestorsOrSelf::Iterator::Iterator(Node* node) : node_(node) {}

Node::AncestorsOrSelf::Iterator::Iterator(const Iterator& other)
    : node_(other.node_) {}

Node::AncestorsOrSelf::Iterator::~Iterator() {}

Node* Node::AncestorsOrSelf::Iterator::operator*() const {
  DCHECK(node_);
  return node_;
}

Node* Node::AncestorsOrSelf::Iterator::operator->() const {
  DCHECK(node_);
  return node_;
}

Node::AncestorsOrSelf::Iterator& Node::AncestorsOrSelf::Iterator::operator++() {
  DCHECK(node_);
  node_ = node_->parent();
  return *this;
}

bool Node::AncestorsOrSelf::Iterator::operator==(const Iterator& other) const {
  return node_ == other.node_;
}

bool Node::AncestorsOrSelf::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals

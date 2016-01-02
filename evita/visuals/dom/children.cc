// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/children.h"

#include "base/logging.h"
#include "evita/visuals/dom/container_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::Children
//
Node::Children::Children(const ContainerNode& container)
    : container_(&container) {}

Node::Children::Children(const Node::Children& other)
    : container_(other.container_) {}

Node::Children::~Children() {}

Node::Children& Node::Children::operator=(const Node::Children& other) {
  container_ = other.container_;
  return *this;
}

Node::Children::Iterator Node::Children::begin() const {
  return Iterator(container_->first_child());
}

Node::Children::Iterator Node::Children::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Node::Children::Iterator
//
Node::Children::Iterator::Iterator(Node* node) : node_(node) {}

Node::Children::Iterator::Iterator(const Iterator& other)
    : node_(other.node_) {}

Node::Children::Iterator::~Iterator() {}

Node* Node::Children::Iterator::operator*() const {
  DCHECK(node_);
  return node_;
}

Node* Node::Children::Iterator::operator->() const {
  DCHECK(node_);
  return node_;
}

Node::Children::Iterator& Node::Children::Iterator::operator++() {
  DCHECK(node_);
  node_ = node_->next_sibling();
  return *this;
}

bool Node::Children::Iterator::operator==(const Iterator& other) const {
  if (node_ && other.node_)
    DCHECK_EQ(node_->parent(), other.node_->parent());
  return node_ == other.node_;
}

bool Node::Children::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/ancestors.h"

#include "base/logging.h"
#include "evita/visuals/dom/container_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::Ancestors
//
Node::Ancestors::Ancestors(const Node& node) : node_(node.parent()) {}
Node::Ancestors::~Ancestors() {}

Node::Ancestors::Iterator Node::Ancestors::begin() const {
  return Iterator(const_cast<ContainerNode*>(node_));
}

Node::Ancestors::Iterator Node::Ancestors::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Node::Ancestors::Iterator
//
Node::Ancestors::Iterator::Iterator(ContainerNode* node) : node_(node) {}

Node::Ancestors::Iterator::Iterator(const Iterator& other)
    : node_(other.node_) {}

Node::Ancestors::Iterator::~Iterator() {}

ContainerNode* Node::Ancestors::Iterator::operator*() const {
  DCHECK(node_);
  return node_;
}

ContainerNode* Node::Ancestors::Iterator::operator->() const {
  DCHECK(node_);
  return node_;
}

Node::Ancestors::Iterator& Node::Ancestors::Iterator::operator++() {
  DCHECK(node_);
  node_ = node_->parent();
  return *this;
}

bool Node::Ancestors::Iterator::operator==(const Iterator& other) const {
  return node_ == other.node_;
}

bool Node::Ancestors::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals

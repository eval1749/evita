// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_traversal.h"

#include "base/logging.h"
#include "joana/ast/container_node.h"

namespace joana {
namespace ast {

//
// NodeAncestors
//

// |NodeTraversal::AncestorsOf()| may calls |NodeAncestors| constructor with
// |nullptr|.
NodeAncestors::NodeAncestors(const Node* node)
    : node_(const_cast<Node*>(node)) {}

NodeAncestors::NodeAncestors(const NodeAncestors& other) : node_(other.node_) {}

NodeAncestors::~NodeAncestors() = default;

NodeAncestors::Iterator::Iterator(const NodeAncestors* owner, Node* node)
    : node_(node), owner_(owner) {
  DCHECK(owner);
}

NodeAncestors::Iterator::Iterator(const Iterator& other)
    : node_(other.node_), owner_(other.owner_) {}

NodeAncestors::Iterator::~Iterator() = default;

Node& NodeAncestors::Iterator::operator*() const {
  DCHECK(node_);
  return *node_;
}

NodeAncestors::Iterator& NodeAncestors::Iterator::operator++() {
  DCHECK(node_);
  node_ = NodeTraversal::ParentOf(*node_);
  return *this;
}

bool NodeAncestors::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(owner_, other.owner_);
  return node_ == other.node_;
}

bool NodeAncestors::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//
// NodeChildren
//
NodeChildren::NodeChildren(const ContainerNode& container)
    : container_(const_cast<ContainerNode*>(&container)) {}

NodeChildren::NodeChildren(const NodeChildren& other)
    : container_(other.container_) {}

NodeChildren::~NodeChildren() = default;

NodeChildren::Iterator NodeChildren::begin() const {
  return Iterator(this, NodeTraversal::FirstChildOf(*container_));
}

NodeChildren::Iterator::Iterator(const NodeChildren* owner, Node* node)
    : node_(node), owner_(owner) {
  DCHECK(owner);
}

NodeChildren::Iterator::Iterator(const Iterator& other)
    : node_(other.node_), owner_(other.owner_) {}

NodeChildren::Iterator::~Iterator() = default;

Node& NodeChildren::Iterator::operator*() const {
  DCHECK(node_);
  return *node_;
}

NodeChildren::Iterator& NodeChildren::Iterator::operator++() {
  DCHECK(node_);
  node_ = NodeTraversal::NextSiblingOf(*node_);
  return *this;
}

bool NodeChildren::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(owner_, other.owner_);
  return node_ == other.node_;
}

bool NodeChildren::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//
// NodeTraversal
//
NodeAncestors NodeTraversal::AncestorsOf(const Node& node) {
  return NodeAncestors(ParentOf(node));
}

Node& NodeTraversal::ChildAt(const ContainerNode& container, int index) {
  DCHECK_GE(index, 0);
  auto position = 0;
  for (const auto& child : ChildrenOf(container)) {
    if (position == index)
      return const_cast<Node&>(child);
    ++position;
  }
  NOTREACHED() << "Index " << index << " is too large for " << container;
  return const_cast<ContainerNode&>(container);
}

NodeChildren NodeTraversal::ChildrenOf(const ContainerNode& container) {
  return NodeChildren(container);
}

int NodeTraversal::CountChildren(const ContainerNode& container) {
  auto position = 0;
  for (const auto& child : ChildrenOf(container)) {
    static_cast<void>(child);
    ++position;
  }
  return position;
}

Node* NodeTraversal::FirstChildOf(const ContainerNode& container) {
  return container.first_child();
}

NodeAncestors NodeTraversal::InclusiveAncestorsOf(const Node& node) {
  return NodeAncestors(&node);
}

Node* NodeTraversal::LastChildOf(const ContainerNode& container) {
  return container.last_child();
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

}  // namespace ast
}  // namespace joana

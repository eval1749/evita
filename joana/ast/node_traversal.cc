// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node_traversal.h"

#include "base/logging.h"
#include "joana/ast/node.h"

namespace joana {
namespace ast {

//
// ChildNodes::Iterator
//
ChildNodes::Iterator::Iterator(const ChildNodes& owner, size_t index)
    : index_(index), owner_(&owner) {}

ChildNodes::Iterator::Iterator(const Iterator& other)
    : Iterator(*other.owner_, other.index_) {}

ChildNodes::Iterator::~Iterator() = default;

const Node& ChildNodes::Iterator::operator*() const {
  return owner_->container_->child_at(index_);
}

ChildNodes::Iterator& ChildNodes::Iterator::operator++() {
  DCHECK_LT(index_, owner_->container_->arity());
  ++index_;
  return *this;
}

bool ChildNodes::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(owner_, other.owner_);
  return index_ == other.index_;
}

bool ChildNodes::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//
// ChildNodes
//
ChildNodes::ChildNodes(const Node& container, size_t start)
    : container_(&container), start_(start) {
  DCHECK_LE(start_, container_->arity());
}

ChildNodes::~ChildNodes() = default;

ChildNodes::Iterator ChildNodes::begin() const {
  return Iterator(*this, start_);
}

bool ChildNodes::empty() const {
  return size() == 0;
}

ChildNodes::Iterator ChildNodes::end() const {
  return Iterator(*this, container_->arity());
}

size_t ChildNodes::size() const {
  return container_->arity() - start_;
}

//
// InclusiveDescendants::Iterator
//
InclusiveDescendants::Iterator::Iterator(const InclusiveDescendants& owner,
                                         const Node* container,
                                         const Node* start_node)
    : owner_(&owner) {
  if (!container && !start_node)
    return;
  stack_.push(std::make_pair(container, 0));
}

InclusiveDescendants::Iterator::Iterator(Iterator&& other)
    : owner_(other.owner_), stack_(std::move(other.stack_)) {}

InclusiveDescendants::Iterator::~Iterator() = default;

const Node& InclusiveDescendants::Iterator::operator*() const {
  const auto* container = stack_.top().first;
  if (!container)
    return *owner_->start_node_;
  const auto index = stack_.top().second;
  return container->child_at(index);
}

InclusiveDescendants::Iterator& InclusiveDescendants::Iterator::operator++() {
  if (!stack_.top().first) {
    DCHECK_EQ(stack_.top().second, 0);
    stack_.pop();
    const auto* const start_node = owner_->start_node_;
    if (start_node->arity() == 0)
      return *this;
    stack_.push(std::make_pair(start_node, 0));
    return *this;
  }

  const auto* const container = stack_.top().first;
  const auto index = stack_.top().second;
  const auto& child = container->child_at(index);
  if (child.arity() > 0) {
    stack_.push(std::make_pair(&child, 0));
    return *this;
  }
  while (!stack_.empty()) {
    ++stack_.top().second;
    if (stack_.top().second < stack_.top().first->arity())
      return *this;
    stack_.pop();
  }
  return *this;
}

bool InclusiveDescendants::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(owner_, other.owner_);
  if (stack_.empty())
    return other.stack_.empty();
  if (stack_.size() != other.stack_.size())
    return false;
  return stack_.top() == other.stack_.top();
}

bool InclusiveDescendants::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//
// InclusiveDescendants
//
InclusiveDescendants::InclusiveDescendants(const Node* container,
                                           const Node* start_node)
    : container_(container), start_node_(start_node) {
  if (container) {
    DCHECK(!start_node)
        << "start node should be null for descendants generator.";
    return;
  }
  DCHECK(start_node) << "Need start node for inclusive descendants generator.";
}

InclusiveDescendants::~InclusiveDescendants() = default;

InclusiveDescendants::Iterator InclusiveDescendants::begin() const {
  return Iterator(*this, container_, start_node_);
}

InclusiveDescendants::Iterator InclusiveDescendants::end() const {
  return Iterator(*this, nullptr, nullptr);
}

//
// NodeTraversal

ChildNodes NodeTraversal::ChildNodesFrom(const Node& node, size_t index) {
  return ChildNodes(node, index);
}

ChildNodes NodeTraversal::ChildNodesOf(const Node& node) {
  return ChildNodes(node, 0);
}

InclusiveDescendants NodeTraversal::DescendantsOf(const Node& container) {
  return InclusiveDescendants(&container, nullptr);
}

InclusiveDescendants NodeTraversal::InclusiveDescendantsOf(
    const Node& start_node) {
  return InclusiveDescendants(nullptr, &start_node);
}

}  // namespace ast
}  // namespace joana

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
  DCHECK_LT(start_, container_->arity());
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

ChildNodes NodeTraversal::ChildNodesFrom(const Node& node, size_t index) {
  return ChildNodes(node, index);
}
ChildNodes NodeTraversal::ChildNodesOf(const Node& node) {
  return ChildNodes(node, 0);
}

}  // namespace ast
}  // namespace joana

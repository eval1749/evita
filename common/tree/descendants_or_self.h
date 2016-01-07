// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_TREE_DESCENDANTS_OR_SELF_H_
#define COMMON_TREE_DESCENDANTS_OR_SELF_H_

#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"

namespace common {
namespace tree {
namespace internal {

template <typename NodeType>
class DescendantsOrSelf {
 public:
  class Iterator
      : public AbstractNodeIterator<std::input_iterator_tag, NodeType> {
   public:
    Iterator(NodeType* scope, NodeType* node)
        : AbstractNodeIterator(node), scope_(scope) {}

    Iterator& operator++() {
      DCHECK(node_);
      if (auto first_child = node_->first_child()) {
        node_ = first_child;
        return *this;
      }

      while (node_ != scope_) {
        if (auto const next = node_->next_sibling()) {
          node_ = next;
          return *this;
        }
        node_ = node_->parent_node();
      }
      node_ = nullptr;
      return *this;
    }

   private:
    NodeType* scope_;
  };

  explicit DescendantsOrSelf(NodeType* node) : node_(node) {}

  Iterator begin() const { return Iterator(node_, node_); }
  Iterator end() const { return Iterator(node_, nullptr); }

 protected:
  NodeType* node_;
};

}  // namespace internal

template <class NodeClass>
internal::DescendantsOrSelf<const NodeClass> descendants_or_self(
    const NodeClass* node) {
  return internal::DescendantsOrSelf<const NodeClass>(node);
}

template <class NodeClass>
internal::DescendantsOrSelf<NodeClass> descendants_or_self(NodeClass* node) {
  return internal::DescendantsOrSelf<NodeClass>(node);
}

}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_DESCENDANTS_OR_SELF_H_

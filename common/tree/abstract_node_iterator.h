// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_TREE_ABSTRACT_NODE_ITERATOR_H_
#define COMMON_TREE_ABSTRACT_NODE_ITERATOR_H_

#include <iterator>

#include "base/logging.h"
#include "base/macros.h"

namespace common {
namespace tree {
namespace internal {

template <class IteratorCategory, typename NodeType>
class AbstractNodeIterator : public std::iterator<IteratorCategory, NodeType*> {
 public:
  value_type operator*() const {
    DCHECK(node_);
    return node_;
  }

  bool operator==(const AbstractNodeIterator& other) const {
    return node_ == other.node_;
  }

  bool operator!=(const AbstractNodeIterator& other) const {
    return node_ != other.node_;
  }

 protected:
  explicit AbstractNodeIterator(value_type node) : node_(node) {}

  value_type node_;

 private:
  value_type operator->() const {
    DCHECK(node_);
    return node_;
  }
};

}  // namespace internal
}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_ABSTRACT_NODE_ITERATOR_H_

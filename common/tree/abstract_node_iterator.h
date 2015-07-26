// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef COMMON_TREE_ABSTRACT_NODE_ITERATOR_H_
#define COMMON_TREE_ABSTRACT_NODE_ITERATOR_H_

#include <iterator>

#include "base/basictypes.h"
#include "base/logging.h"

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

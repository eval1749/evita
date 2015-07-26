// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef COMMON_TREE_ANCESTORS_OR_SELF_H_
#define COMMON_TREE_ANCESTORS_OR_SELF_H_

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"

namespace common {
namespace tree {
namespace internal {

template <typename NodeType>
class AncestorsOrSelf {
 public:
  class Iterator
      : public AbstractNodeIterator<std::input_iterator_tag, NodeType> {
   public:
    explicit Iterator(NodeType* node) : AbstractNodeIterator(node) {}

    Iterator& operator++() {
      DCHECK(node_);
      node_ = node_->parent_node();
      return *this;
    }
  };

  explicit AncestorsOrSelf(NodeType* node) : node_(node) {}

  Iterator begin() const { return Iterator(node_); }
  Iterator end() const { return Iterator(nullptr); }

 private:
  NodeType* node_;
};

}  // namespace internal

template <class NodeClass>
internal::AncestorsOrSelf<const NodeClass> ancestors_or_self(
    const NodeClass* node) {
  return internal::AncestorsOrSelf<const NodeClass>(node);
}

template <class NodeClass>
internal::AncestorsOrSelf<NodeClass> ancestors_or_self(NodeClass* node) {
  return internal::AncestorsOrSelf<NodeClass>(node);
}

}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_ANCESTORS_OR_SELF_H_

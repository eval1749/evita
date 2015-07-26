// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_TREE_DESCENDANTS_H_
#define COMMON_TREE_DESCENDANTS_H_

#include "common/tree/descendants_or_self.h"

#include "base/basictypes.h"

namespace common {
namespace tree {
namespace internal {

template <typename NodeType>
class Descendants : public DescendantsOrSelf<NodeType> {
 public:
  explicit Descendants(NodeType* node) : DescendantsOrSelf(node) {}
  Iterator begin() const { return Iterator(node_, node_->first_child()); }
  Iterator end() const { return Iterator(node_, nullptr); }
};

}  // namespace internal

template <class NodeClass>
internal::Descendants<const NodeClass> descendants(const NodeClass* node) {
  return internal::Descendants<const NodeClass>(node);
}

template <class NodeClass>
internal::Descendants<NodeClass> descendants(NodeClass* node) {
  return internal::Descendants<NodeClass>(node);
}

template <class NodeClass>
bool Node<NodeClass>::Contains(const NodeClass* node) const {
  for (auto child : descendants(this)) {
    if (node == child)
      return true;
  }
  return false;
}

}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_DESCENDANTS_H_

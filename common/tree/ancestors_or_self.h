// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_ancestors_or_slef_h)
#define INCLUDE_common_tree_ancestors_or_slef_h

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"

namespace common {
namespace tree {
namespace internal {
template<typename NodeType>
class AncestorsOrSelf {
  public: class Iterator
      : public AbstractNodeIterator<std::input_iterator_tag, NodeType> {
    public: Iterator(NodeType* node) : AbstractNodeIterator(node) {
    }
    public: Iterator& operator++() {
      DCHECK(node_);
      node_ = node_->parent_node();
      return *this;
    }
  };

  private: NodeType* node_;

  public: explicit AncestorsOrSelf(NodeType* node) : node_(node) {
  }

  public: Iterator begin() const { return Iterator(node_); }
  public: Iterator end() const { return Iterator(nullptr); }

  DISALLOW_COPY_AND_ASSIGN(AncestorsOrSelf);
};
} // naemspace internal

template<class NodeClass>
internal::AncestorsOrSelf<const NodeClass>
ancestors_or_self(const NodeClass* node) {
  return internal::AncestorsOrSelf<const NodeClass>(node);
}

template<class NodeClass>
internal::AncestorsOrSelf<NodeClass>
ancestors_or_self(NodeClass* node) {
  return internal::AncestorsOrSelf<NodeClass>(node);
}

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_ancestors_or_slef_h)

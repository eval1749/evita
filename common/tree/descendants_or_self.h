// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_descendants_or_self_h)
#define INCLUDE_common_tree_descendants_or_self_h

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"

namespace common {
namespace tree {
namespace internal {

template<typename NodeType>
class DescendantsOrSelf {
  public: class Iterator
      : public AbstractNodeIterator<std::input_iterator_tag, NodeType> {
    private: NodeType* scope_;
    public: Iterator(NodeType* scope, NodeType* node)
        : AbstractNodeIterator(node), scope_(scope) {
    }
    public: Iterator& operator++() {
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
  };

  protected: NodeType* node_;

  public: explicit DescendantsOrSelf(NodeType* node) : node_(node) {
  }

  public: Iterator begin() const { return Iterator(node_, node_); }
  public: Iterator end() const { return Iterator(node_, nullptr); }

  DISALLOW_COPY_AND_ASSIGN(DescendantsOrSelf);
};

} // naemspace internal

template<class NodeClass>
internal::DescendantsOrSelf<const NodeClass>
descendants_or_self(const NodeClass* node) {
  return internal::DescendantsOrSelf<const NodeClass>(node);
}

template<class NodeClass>
internal::DescendantsOrSelf<NodeClass>
descendants_or_self(NodeClass* node) {
  return internal::DescendantsOrSelf<NodeClass>(node);
}

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_descendants_or_self_h)

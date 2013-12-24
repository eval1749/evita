// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_descendants_or_self_h)
#define INCLUDE_common_tree_descendants_or_self_h

#include "common/tree/abstract_node_iterator.h"

namespace common {
namespace tree {
namespace impl {

template<typename NodeType>
class DescendantsOrSelf_ {
  protected: typedef NodeType* PtrType;
  protected: typedef NodeType& RefType;
  public: class Iterator
      : public AbstractNodeIterator_<std::input_iterator_tag, NodeType> {
    private: NodeType* scope_;
    public: Iterator(PtrType scope, PtrType node)
        : AbstractNodeIterator_(node), scope_(scope) {
    }
    public: Iterator& operator++() {
      ASSERT(node_);
      auto const container = node_->ToContainer();
      if (auto first_child = container ? container->first_child() : nullptr) {
        node_ = container->first_child();
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

  protected: RefType node_;

  public: explicit DescendantsOrSelf_(RefType node) : node_(node) {
  }

  public: Iterator begin() const { return Iterator(&node_, &node_); }
  public: Iterator end() const { return Iterator(&node_, nullptr); }

  DISALLOW_COPY_AND_ASSIGN(DescendantsOrSelf_);
};

} // naemspace impl

template<class NodeClass>
impl::DescendantsOrSelf_<const typename NodeClass::Node>
descendants_or_self(const NodeClass& node) {
  return impl::DescendantsOrSelf_<const NodeClass::Node>(node);
}

template<class NodeClass>
impl::DescendantsOrSelf_<typename NodeClass::Node>
descendants_or_self(NodeClass& node) {
  return impl::DescendantsOrSelf_<NodeClass::Node>(node);
}

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_descendants_or_self_h)

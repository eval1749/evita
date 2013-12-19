// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_tree_ancestors_or_slef_h)
#define INCLUDE_base_tree_ancestors_or_slef_h

#include "base/tree/abstract_node_iterator.h"

namespace base {
namespace tree {
namespace impl {
template<typename NodeType>
class AncestorsOrSelf_ {
  private: typedef NodeType* PtrType;
  private: typedef NodeType& RefType;
  public: class Iterator
      : public AbstractNodeIterator_<std::input_iterator_tag, NodeType> {
    public: Iterator(PtrType node)
        : AbstractNodeIterator_(node) {
    }
    public: Iterator& operator++() {
      ASSERT(node_);
      node_ = node_->parent_node();
      return *this;
    }
  };

  private: RefType node_;

  public: explicit AncestorsOrSelf_(RefType node) : node_(node) {
  }

  public: Iterator begin() const { return Iterator(&node_); }
  public: Iterator end() const { return Iterator(nullptr); }

  DISALLOW_COPY_AND_ASSIGN(AncestorsOrSelf_);
};
} // naemspace impl

template<typename ContainerType>
impl::AncestorsOrSelf_<const ContainerType>
ancestors_or_self(const ContainerType& container) {
  return impl::AncestorsOrSelf_<const ContainerType>(container);
}

template<typename ContainerType>
impl::AncestorsOrSelf_<ContainerType>
ancestors_or_self(ContainerType& container) {
  return impl::AncestorsOrSelf_<ContainerType>(container);
}

} // namespace tree
} // namespace base

#endif //!defined(INCLUDE_base_tree_ancestors_or_slef_h)

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_tree_abstract_node_iterator_h)
#define INCLUDE_base_tree_abstract_node_iterator_h

#include <iterator>

namespace base {
namespace tree {
namespace impl {

template<class IteratorCategory, typename NodeType>
class AbstractNodeIterator_
    : public std::iterator<IteratorCategory, NodeType> {
  protected: typedef NodeType* PtrNodeType;
  protected: typedef NodeType& RefNodeType;
  protected: PtrNodeType node_;

  protected: explicit AbstractNodeIterator_(PtrNodeType node)
      : node_(node) {
  }

  public: operator RefNodeType() const {
    ASSERT(node_);
    return *node_;
  }

  public: RefNodeType operator*() const {
    ASSERT(node_);
    return *node_;
  }

  public: PtrNodeType operator->() const {
    ASSERT(node_);
    return node_;
  }

  public: bool operator==(const AbstractNodeIterator_& other) const {
    return node_ == other.node_;
  }

  public: bool operator!=(const AbstractNodeIterator_& other) const {
    return node_ != other.node_;
  }
};

} // namespace impl
} // namespace tree
} // namespace base

#endif //!defined(INCLUDE_base_tree_abstract_node_iterator_h)

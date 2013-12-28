// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_abstract_node_iterator_h)
#define INCLUDE_common_tree_abstract_node_iterator_h

#include <iterator>

#include "base/basictypes.h"
#include "base/logging.h"

namespace common {
namespace tree {
namespace internal {

template<class IteratorCategory, typename NodeType>
class AbstractNodeIterator
    : public std::iterator<IteratorCategory, NodeType*> {
  protected: value_type node_;

  protected: explicit AbstractNodeIterator(value_type node)
      : node_(node) {
  }

  public: value_type operator*() const {
    DCHECK(node_);
    return node_;
  }

  private: value_type operator->() const {
    DCHECK(node_);
    return node_;
  }

  public: bool operator==(const AbstractNodeIterator& other) const {
    return node_ == other.node_;
  }

  public: bool operator!=(const AbstractNodeIterator& other) const {
    return node_ != other.node_;
  }

  DISALLOW_COPY_AND_ASSIGN(AbstractNodeIterator);
};

} // namespace internal
} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_abstract_node_iterator_h)

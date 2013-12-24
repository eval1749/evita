// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_child_nodes_h)
#define INCLUDE_common_tree_child_nodes_h

#include "common/tree/abstract_node_iterator.h"
#include "common/tree/container_node.h"

namespace common {
namespace tree {

template<typename ContainerType>
class ChildNodes_ {
  public: typedef typename ContainerType::Node NodeType;
  private: class Iterator
      : public impl::AbstractNodeIterator_<std::bidirectional_iterator_tag,
                                           NodeType> {
    protected: Iterator(NodeType* node) : AbstractNodeIterator_(node) {
    }
    protected: void Decrement() {
      ASSERT(node_);
      node_ = node_->previous_sibling();
    }
    protected: void Increment() {
      ASSERT(node_);
      node_ = node_->next_sibling();
    }
  };
  public: class BackwardIterator : public Iterator {
    public: explicit BackwardIterator(NodeType* node) : Iterator(node) {
    }
    public: BackwardIterator& operator++() {
      Decrement();
      return *this;
    }
    public: BackwardIterator& operator--() {
      Increment();
      return *this;
    }
  };
  public: class ForwardIterator : public Iterator {
    public: explicit ForwardIterator(NodeType* node) : Iterator(node) {
    }
    public: ForwardIterator& operator++() {
      Increment();
      return *this;
    }
    public: ForwardIterator& operator--() {
      Decrement();
      return *this;
    }
  };

  private: ContainerType& container_;

  public: explicit ChildNodes_(ContainerType& container)
      : container_(container) {
  }

  public: ForwardIterator begin() const {
    return ForwardIterator(container_.first_child());
  }
  public: ForwardIterator end() const {
    return ForwardIterator(nullptr);
  }
  // TODO: We should have ConstForwardIterator.
  public:ForwardIterator cbegin() const {
    return ForwardIterator(container_.first_child());
  }
  // TODO: We should have ConstForwardIterator.
  public:ForwardIterator cend() const {
    return ConstForwardIterator(nullptr);
  }
  public: BackwardIterator rbegin() const {
    return BackwardIterator(container_.last_child());
  }
  public: BackwardIterator rend() const {
    return BackwardIterator(nullptr);
  }
  // TODO: We should have ConstBackwardIterator.
  public:BackwardIterator crbegin() const {
    return BackwardIterator(container_.last_child());
  }
  // TODO: We should have ConstBackwardIterator.
  public:BackwardIterator crend() const {
    return BackwardIterator(nullptr);
  }

  DISALLOW_COPY_AND_ASSIGN(ChildNodes_);
};

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_child_nodes_h)

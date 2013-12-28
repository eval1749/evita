// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_child_nodes_h)
#define INCLUDE_common_tree_child_nodes_h

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"
#include "common/tree/node.h"

namespace common {
namespace tree {

template<typename NodeType>
class ChildNodes {
  private: class Iterator
      : public internal::AbstractNodeIterator<std::bidirectional_iterator_tag,
                                              NodeType> {
    protected: Iterator(NodeType* node) : AbstractNodeIterator(node) {
    }
    protected: void Decrement() {
      DCHECK(node_);
      node_ = node_->previous_sibling();
    }
    protected: void Increment() {
      DCHECK(node_);
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

  private: NodeType* parent_node_;

  public: explicit ChildNodes(NodeType* parent_node)
      : parent_node_(parent_node) {
  }

  public: ForwardIterator begin() const {
    return ForwardIterator(parent_node_->first_child());
  }
  public: ForwardIterator end() const {
    return ForwardIterator(nullptr);
  }
  // TODO: We should have ConstForwardIterator.
  public:ForwardIterator cbegin() const {
    return ForwardIterator(parent_node_->first_child());
  }
  // TODO: We should have ConstForwardIterator.
  public:ForwardIterator cend() const {
    return ConstForwardIterator(nullptr);
  }
  public: BackwardIterator rbegin() const {
    return BackwardIterator(parent_node_->last_child());
  }
  public: BackwardIterator rend() const {
    return BackwardIterator(nullptr);
  }
  // TODO: We should have ConstBackwardIterator.
  public:BackwardIterator crbegin() const {
    return BackwardIterator(parent_node_->last_child());
  }
  // TODO: We should have ConstBackwardIterator.
  public:BackwardIterator crend() const {
    return BackwardIterator(nullptr);
  }

  DISALLOW_COPY_AND_ASSIGN(ChildNodes);
};

template<class NodeClass>
ChildNodes<const NodeClass> Node<NodeClass>::child_nodes() const {
  return ChildNodes<const NodeClass>(static_cast<const NodeClass*>(this));
}

template<class NodeClass>
ChildNodes<NodeClass> Node<NodeClass>::child_nodes() {
  return ChildNodes<NodeClass>(static_cast<NodeClass*>(this));
}


} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_child_nodes_h)

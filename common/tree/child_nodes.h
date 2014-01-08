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
  private: template<typename T> class Iterator
      : public internal::AbstractNodeIterator<std::bidirectional_iterator_tag,
                                              NodeType> {
    protected: Iterator(T* node) : AbstractNodeIterator(node) {
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
  public: class iterator : public Iterator<NodeType> {
    public: explicit iterator(NodeType* node) : Iterator(node) {
    }
    public: iterator& operator++() {
      Increment();
      return *this;
    }
    public: iterator& operator--() {
      Decrement();
      return *this;
    }
  };
  public: class const_iterator : public Iterator<const NodeType> {
    public: explicit const_iterator(const NodeType* node) : Iterator(node) {
    }
    public: const_iterator& operator++() {
      Increment();
      return *this;
    }
    public: const_iterator& operator--() {
      Decrement();
      return *this;
    }
  };
  public: class reverse_iterator : public Iterator<NodeType> {
    public: explicit reverse_iterator(NodeType* node) : Iterator(node) {
    }
    public: reverse_iterator(const reverse_iterator& other)
        : Iterator(other.node_) {
    }
    public: iterator& operator++() {
      Decrement();
      return *this;
    }
    public: iterator& operator--() {
      Increment();
      return *this;
    }
  };
  public: class const_reverse_iterator : public Iterator<NodeType> {
    public: explicit const_reverse_iterator(const NodeType* node)
        : Iterator(node) {
    }
    public: const_reverse_iterator(const const_reverse_iterator& other)
        : Iterator(other.node_) {
    }
    public: iterator& operator++() {
      Decrement();
      return *this;
    }
    public: iterator& operator--() {
      Increment();
      return *this;
    }
  };

  private: NodeType* parent_node_;

  public: explicit ChildNodes(NodeType* parent_node)
      : parent_node_(parent_node) {
  }

  // iterrator and reverse_iterrator
  public: iterator begin() {
    return iterator(parent_node_->first_child());
  }
  public: iterator end() { return iterator(nullptr); }
  public: reverse_iterator rbegin() {
    return reverse_iterator(parent_node_->last_child());
  }
  public: reverse_iterator rend() { return reverse_iterator(nullptr); }

  // const_iterrator and const_reverse_iterrator
  public: const_iterator begin() const { return cbegin(); }
  public: const_iterator end() const { return cend(); }
  public: const_reverse_iterator rbegin() const { return crbegin(); }
  public: const_reverse_iterator rend() const { return crend(); }

  // const_iterrator and const_reverse_iterrator
  public: const_iterator cbegin() const {
    return const_iterator(parent_node_->first_child());
  }
  public: const_iterator cend() const {
    return const_iterator(nullptr);
  }
  public: const_reverse_iterator crbegin() const {
    return const_reverse_iterator(parent_node_->last_child());
  }
  public: const_reverse_iterator crend() const {
    return const reverse_iterator(nullptr);
  }

  DISALLOW_COPY_AND_ASSIGN(ChildNodes);
};

template<class NodeClass>
ChildNodes<const NodeClass> Node<NodeClass>::child_nodes() const {
  return ChildNodes<const NodeClass>(
      const_cast<NodeClass*>(static_cast<const NodeClass*>(this)));
}

template<class NodeClass>
ChildNodes<NodeClass> Node<NodeClass>::child_nodes() {
  return ChildNodes<NodeClass>(static_cast<NodeClass*>(this));
}

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_child_nodes_h)

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_TREE_CHILD_NODES_H_
#define COMMON_TREE_CHILD_NODES_H_

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/tree/abstract_node_iterator.h"
#include "common/tree/node.h"

namespace common {
namespace tree {

template <typename NodeType>
class ChildNodes {
 private:
  template <typename T>
  class Iterator
      : public internal::AbstractNodeIterator<std::bidirectional_iterator_tag,
                                              NodeType> {
   protected:
    explicit Iterator(T* node) : AbstractNodeIterator(node) {}

    void Decrement() {
      DCHECK(node_);
      node_ = node_->previous_sibling();
    }

    void Increment() {
      DCHECK(node_);
      node_ = node_->next_sibling();
    }
  };

 public:
  class iterator : public Iterator<NodeType> {
   public:
    explicit iterator(NodeType* node) : Iterator(node) {}

   public:
    iterator& operator++() {
      Increment();
      return *this;
    }

    iterator& operator--() {
      Decrement();
      return *this;
    }
  };

  class const_iterator : public Iterator<const NodeType> {
   public:
    explicit const_iterator(const NodeType* node) : Iterator(node) {}

    const_iterator& operator++() {
      Increment();
      return *this;
    }

    const_iterator& operator--() {
      Decrement();
      return *this;
    }
  };

  class reverse_iterator : public Iterator<NodeType> {
   public:
    explicit reverse_iterator(NodeType* node) : Iterator(node) {}
    reverse_iterator(const reverse_iterator& other) : Iterator(other.node_) {}

    iterator& operator++() {
      Decrement();
      return *this;
    }

    iterator& operator--() {
      Increment();
      return *this;
    }
  };

  class const_reverse_iterator : public Iterator<NodeType> {
   public:
    explicit const_reverse_iterator(const NodeType* node) : Iterator(node) {}
    const_reverse_iterator(const const_reverse_iterator& other)
        : Iterator(other.node_) {}

    iterator& operator++() {
      Decrement();
      return *this;
    }

    iterator& operator--() {
      Increment();
      return *this;
    }
  };

  explicit ChildNodes(NodeType* parent_node) : parent_node_(parent_node) {}

  // iterator and reverse_iterator
  iterator begin() { return iterator(parent_node_->first_child()); }
  iterator end() { return iterator(nullptr); }
  reverse_iterator rbegin() {
    return reverse_iterator(parent_node_->last_child());
  }
  reverse_iterator rend() { return reverse_iterator(nullptr); }

  // const_iterator and const_reverse_iterator
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }
  const_reverse_iterator rbegin() const { return crbegin(); }
  const_reverse_iterator rend() const { return crend(); }

  // const_iterator and const_reverse_iterator
  const_iterator cbegin() const {
    return const_iterator(parent_node_->first_child());
  }
  const_iterator cend() const { return const_iterator(nullptr); }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(parent_node_->last_child());
  }
  const_reverse_iterator crend() const {
    return const reverse_iterator(nullptr);
  }

 private:
  NodeType* parent_node_;
};

template <class NodeClass>
ChildNodes<const NodeClass> Node<NodeClass>::child_nodes() const {
  return ChildNodes<const NodeClass>(
      const_cast<NodeClass*>(static_cast<const NodeClass*>(this)));
}

template <class NodeClass>
ChildNodes<NodeClass> Node<NodeClass>::child_nodes() {
  return ChildNodes<NodeClass>(static_cast<NodeClass*>(this));
}

}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_CHILD_NODES_H_

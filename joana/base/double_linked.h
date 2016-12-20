// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_DOUBLE_LINKED_H_
#define JOANA_BASE_DOUBLE_LINKED_H_

#include <iterator>

#include "base/logging.h"
#include "base/macros.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// DoubleLinked
// provides O(1) insertion and deletion of node in list. A node of list must
// be node from |DoubleLinked<T, U>::NodeBase| with |public| accessibility.
//
// Example:
//  class MyItem : public DoubleLinked<MyItem, MyItemCollection> {
//    ...
//  };
//
//  class MyItemCollection {
//    ...
//   private:
//    DoubleLinked<MyItem, MyItemCollection> items_;
//  };
//
//  Following functions are available:
//    begin()
//    empty()
//    end()
//    first_node()
//    last_node()
//    rbegin()
//    rend()
//    reversed() for last to first iteration in ranged-for loop.
//    AppendNode(Node* new_node)
//    InsertAfter(Node* new_node, Node* ref_node)
//    InsertBefore(Node* new_node, Node* ref_node)
//    PrependNode(Node* new_node)
//    RemoveAll()
//    RemoveNode(Node* old_node)
//    ReplaceNode(Node* new_node, Node* old_node);
//
//  Note:
//    |old_base| must be in a list. |ref_node| must be in a list or null.
//    Unlike DOM API |new_base| must be not in a list.
//
template <typename Node, typename AnchorType>
class DoubleLinked final {
 public:
  // NodeBase
  class NodeBase {
   public:
    Node* next() const { return next_; }
    Node* previous() const { return previous_; }

   protected:
#if DCHECK_IS_ON()
    NodeBase() : next_(nullptr), owner_(nullptr), previous_(nullptr) {}
#else
    NodeBase() : next_(nullptr), previous_(nullptr) {}
#endif
    ~NodeBase() = default;

   private:
    friend class DoubleLinked;

    Node* next_;
    Node* previous_;
#if DCHECK_IS_ON()
    DoubleLinked* owner_;
#endif

    DISALLOW_COPY_AND_ASSIGN(NodeBase);
  };

  class Iterator final : public IteratorBase {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = Node;
    using pointer = Node*;
    using reference = Node&;

    explicit Iterator(Node* node) : IteratorBase(node) {}
    Iterator(const Iterator& other) = default;
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    Iterator& operator++() {
      DCHECK(!!current_);
      current_ = static_cast<NodeBase*>(current_)->next_;
      return *this;
    }
  };

  class ReverseIterator final : public IteratorBase {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = Node;
    using pointer = Node*;
    using reference = Node&;

    explicit ReverseIterator(Node* node) : IteratorBase(node) {}
    ReverseIterator(const ReverseIterator& other) = default;
    ~ReverseIterator() = default;

    ReverseIterator& operator=(const ReverseIterator& other) = default;

    ReverseIterator& operator++() {
      DCHECK(!!current_);
      current_ = static_cast<NodeBase*>(current_)->previous_;
      return *this;
    }
  };

  DoubleLinked() : first_(nullptr), last_(nullptr) {}
  ~DoubleLinked() = default;

  Iterator begin() const { return Iterator(first_); }
  bool empty() const { return !first_; }
  Iterator end() const { return Iterator(nullptr); }
  Node* first_node() const { return first_; }
  Node* last_node() const { return last_; }
  ReverseIterator rbegin() const { return ReverseIterator(last_); }
  ReverseIterator rend() const { return ReverseIterator(nullptr); }

  // Inserts |new_node| at the end of this list. |new_node| must not be
  // in this list.
  void AppendNode(Node* new_node) {
    auto const new_base = static_cast<NodeBase*>(new_node);
#if DCHECK_IS_ON()
    DCHECK(!new_base->owner_) << "new node should not be in this list.";
    new_base->owner_ = this;
#endif
    DCHECK(!new_base->next_);
    DCHECK(!new_base->previous_);
    DCHECK_NE(first_, new_base);

    new_base->next_ = nullptr;
    new_base->previous_ = last_;
    if (!first_)
      first_ = new_node;
    if (last_)
      static_cast<NodeBase*>(last_)->next_ = new_node;
    last_ = new_node;
  }

  // Returns number of elements in this list. This takes O(n), where n is
  // number of node in this list.
  int Count() const {
    auto num_nodes = 0;
    for (auto it = begin(); it != end(); ++it)
      ++num_nodes;
    return num_nodes;
  }

  // Inserts |new_node| after |ref_node|. |new_node| must not be in
  // this list.
  void InsertAfter(Node* new_node, Node* ref_node) {
    DCHECK_NE(new_node, ref_node);

    if (!ref_node) {
      PrependNode(new_node);
      return;
    }
    auto const new_base = static_cast<NodeBase*>(new_node);
#if DCHECK_IS_ON()
    DCHECK(!new_base->owner_) << "new node should not be in this list.";
    new_base->owner_ = this;
#endif
    DCHECK(!new_base->next_);
    DCHECK(!new_base->previous_);
    DCHECK_NE(first_, new_base);

    auto const ref_base = static_cast<NodeBase*>(ref_node);
#if DCHECK_IS_ON()
    DCHECK_EQ(this, ref_base->owner_) << "ref node must be in this list.";
    new_base->owner_ = this;
#endif

    auto const next = ref_base->next_;
    if (next)
      static_cast<NodeBase*>(next)->previous_ = new_node;
    else
      last_ = new_node;

    new_base->next_ = next;
    new_base->previous_ = ref_node;
    ref_base->next_ = new_node;
  }

  // Inserts |new_node| before |ref_node|. |new_node| must not be in
  // this list.
  void InsertBefore(Node* new_node, Node* ref_node) {
    DCHECK_NE(new_node, ref_node);

    if (!ref_node) {
      AppendNode(new_node);
      return;
    }
    auto const new_base = static_cast<NodeBase*>(new_node);
#if DCHECK_IS_ON()
    DCHECK(!new_base->owner_) << "new node should not be in this list.";
    new_base->owner_ = this;
#endif
    DCHECK(!new_base->next_);
    DCHECK(!new_base->previous_);
    DCHECK_NE(first_, new_base);

    auto const ref_base = static_cast<NodeBase*>(ref_node);
#if DCHECK_IS_ON()
    DCHECK_EQ(this, ref_base->owner_) << "ref node must be in this list.";
#endif

    auto const previous = ref_base->previous_;
    if (previous)
      static_cast<NodeBase*>(previous)->next_ = new_node;
    else
      first_ = new_node;

    new_base->next_ = ref_node;
    new_base->previous_ = previous;
    ref_base->previous_ = new_node;
  }

  // Inserts |new_node| at first.
  void PrependNode(Node* new_node) {
    auto const new_base = static_cast<NodeBase*>(new_node);
#if DCHECK_IS_ON()
    DCHECK(!new_base->owner_) << "new node should not be in this list.";
    new_base->owner_ = this;
#endif
    DCHECK(!new_base->next_) << "new node should not be in this list.";
    DCHECK(!new_base->previous_);
    DCHECK_NE(first_, new_base);

    new_base->next_ = first_;
    new_base->previous_ = nullptr;
    if (!last_)
      last_ = new_node;
    if (first_)
      static_cast<NodeBase*>(first_)->previous_ = new_node;
    first_ = new_node;
  }

  // Remove all nodes in this list.
  void RemoveAll() {
    while (first_)
      RemoveNode(first_);
  }

  // Removes |old_node| from this list.
  void RemoveNode(Node* old_node) {
    auto const old_base = static_cast<NodeBase*>(old_node);
#if DCHECK_IS_ON()
    DCHECK_EQ(this, old_base->owner_) << "old_base must be in this list.";
    old_base->owner_ = nullptr;
#endif
    auto const next = old_base->next_;
    auto const previous = old_base->previous_;

    if (next)
      static_cast<NodeBase*>(next)->previous_ = previous;
    else
      last_ = previous;

    if (previous)
      static_cast<NodeBase*>(previous)->next_ = next;
    else
      first_ = next;

    old_base->next_ = nullptr;
    old_base->previous_ = nullptr;
  }

  // Replaces |old_node| by |new_node|. |new_node| should not be in
  // this list.
  void ReplaceNode(Node* new_node, Node* old_node) {
    DCHECK_NE(new_node, old_node);

    auto const new_base = static_cast<NodeBase*>(new_node);
#if DCHECK_IS_ON()
    DCHECK(!new_base->owner_) << "new node should not be in this list.";
    new_base->owner_ = this;
#endif
    DCHECK(!new_base->next_) << "new node should not be in this list.";
    DCHECK(!new_base->previous_);
    DCHECK_NE(first_, new_base);

    auto const old_base = static_cast<NodeBase*>(old_node);
#if DCHECK_IS_ON()
    DCHECK_EQ(this, old_base->owner_) << "old node must be in this list.";
    old_base->owner_ = nullptr;
#endif

    auto const next = old_base->next_;
    auto const previous = old_base->previous_;
    old_base->next_ = nullptr;
    old_base->previous_ = nullptr;

    if (next)
      static_cast<NodeBase*>(next)->previous_ = new_node;
    else
      last_ = new_node;
    new_base->next_ = next;

    if (previous)
      static_cast<NodeBase*>(previous)->next_ = new_node;
    else
      first_ = new_node;

    new_base->previous_ = previous;
  }

 private:
  class IteratorBase {
   public:
    Node* operator*() const { return current_; }
    Node* operator->() const { return current_; }
    bool operator==(const IteratorBase& other) const {
      return current_ == other.current_;
    }
    bool operator!=(const IteratorBase& other) const {
      return !operator==(other);
    }

   protected:
    explicit IteratorBase(Node* node) : current_(node) {}
    ~IteratorBase() = default;

    Node* current_;
  };

  Node* first_;
  Node* last_;

  DISALLOW_COPY_AND_ASSIGN(DoubleLinked);
};

}  // namespace joana

#endif  // JOANA_BASE_DOUBLE_LINKED_H_

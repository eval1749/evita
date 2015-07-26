// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_TREE_NODE_H_
#define COMMON_TREE_NODE_H_

#include "base/basictypes.h"
#include "base/logging.h"

namespace common {
namespace tree {

template <class NodeClas>
class ChildNodes;

template <class NodeClass>
class Node {
 public:
  Node();
  ~Node() = default;

  bool operator==(const NodeClass& other) const;
  bool operator==(const NodeClass* other) const;
  bool operator!=(const NodeClass& other) const;
  bool operator!=(const NodeClass* other) const;

  ChildNodes<const NodeClass> child_nodes() const;
  ChildNodes<NodeClass> child_nodes();
  NodeClass* first_child() const { return first_child_; }
  NodeClass* last_child() const { return last_child_; }
  NodeClass* next_sibling() const { return next_sibling_; }
  NodeClass* parent_node() const { return parent_node_; }
  NodeClass* previous_sibling() const { return previous_sibling_; }

  void AppendChild(NodeClass* node);
  bool Contains(const NodeClass*) const;
  void InsertAfter(NodeClass* node, NodeClass* ref_node);
  void InsertBefore(NodeClass* node, NodeClass* ref_node);
  void PrependChild(NodeClass* node);
  void RemoveChild(NodeClass* node);
  void ReplaceChild(NodeClass* new_node, NodeClass* old_node);

 private:
  NodeClass* first_child_;
  NodeClass* last_child_;
  NodeClass* next_sibling_;
  NodeClass* parent_node_;
  NodeClass* previous_sibling_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

// See common/tree/child_nodes.h for Node::child_nodes() implementation.
// See common/tree/descendant.h for Node::Contains() implementation.

template <class NodeClass>
Node<NodeClass>::Node()
    : first_child_(nullptr),
      last_child_(nullptr),
      next_sibling_(nullptr),
      parent_node_(nullptr),
      previous_sibling_(nullptr) {}

template <class NodeClass>
bool Node<NodeClass>::operator==(const NodeClass& other) const {
  return this == &other;
}

template <class NodeClass>
bool Node<NodeClass>::operator==(const NodeClass* other) const {
  return this == other;
}

template <class NodeClass>
bool Node<NodeClass>::operator!=(const NodeClass& other) const {
  return this != &other;
}

template <class NodeClass>
bool Node<NodeClass>::operator!=(const NodeClass* other) const {
  return this != other;
}

template <class NodeClass>
void Node<NodeClass>::AppendChild(NodeClass* node) {
  if (auto const old_parent = node->parent_node_)
    old_parent->RemoveChild(node);
  DCHECK(!node->parent_node());
  DCHECK(!node->next_sibling());
  DCHECK(!node->previous_sibling());
  node->previous_sibling_ = last_child_;
  if (last_child_) {
    DCHECK(first_child_);
    last_child_->next_sibling_ = node;
  } else {
    DCHECK(!first_child_);
    first_child_ = node;
  }
  last_child_ = node;
  node->parent_node_ = static_cast<NodeClass*>(this);
}

template <class NodeClass>
void Node<NodeClass>::InsertAfter(NodeClass* node, NodeClass* ref_node) {
  if (!ref_node) {
    PrependChild(node);
    return;
  }
  DCHECK_NE(node, ref_node);
  DCHECK_EQ(ref_node->parent_node(), this);
  if (auto const old_parent = node->parent_node_)
    old_parent->RemoveChild(node);
  auto const next = ref_node->next_sibling();
  if (next) {
    next->previous_sibling_ = node;
  } else {
    DCHECK_EQ(last_child_, ref_node);
    last_child_ = node;
  }
  ref_node->next_sibling_ = node;
  node->next_sibling_ = next;
  node->previous_sibling_ = ref_node;
  node->parent_node_ = static_cast<NodeClass*>(this);
}

template <class NodeClass>
void Node<NodeClass>::InsertBefore(NodeClass* node, NodeClass* ref_node) {
  if (!ref_node) {
    AppendChild(node);
    return;
  }
  DCHECK_NE(node, ref_node);
  DCHECK_EQ(ref_node->parent_node_, this);
  if (auto const old_parent = node->parent_node_)
    old_parent->RemoveChild(node);
  auto const previous = ref_node->previous_sibling_;
  if (previous) {
    previous->next_sibling_ = node;
  } else {
    DCHECK_EQ(first_child_, ref_node);
    first_child_ = node;
  }
  ref_node->previous_sibling_ = node;
  node->next_sibling_ = ref_node;
  node->previous_sibling_ = previous;
  node->parent_node_ = static_cast<NodeClass*>(this);
}

template <class NodeClass>
void Node<NodeClass>::PrependChild(NodeClass* node) {
  if (auto const old_parent = node->parent_node_)
    old_parent->RemoveChild(node);
  DCHECK(!node->parent_node());
  DCHECK(!node->next_sibling());
  DCHECK(!node->previous_sibling());
  node->next_sibling_ = first_child_;
  if (first_child_) {
    DCHECK(last_child_);
    first_child_->previous_sibling_ = node;
  } else {
    DCHECK(!last_child_);
    last_child_ = node;
  }
  first_child_ = node;
  node->parent_node_ = static_cast<NodeClass*>(this);
}

template <class NodeClass>
void Node<NodeClass>::RemoveChild(NodeClass* node) {
  DCHECK_EQ(node->parent_node_, this);
  auto const previous = node->previous_sibling();
  auto const next = node->next_sibling();
  if (previous) {
    previous->next_sibling_ = next;
  } else {
    DCHECK_EQ(first_child_, node);
    first_child_ = next;
  }
  if (next) {
    next->previous_sibling_ = previous;
  } else {
    DCHECK_EQ(last_child_, node);
    last_child_ = previous;
  }
  node->next_sibling_ = nullptr;
  node->parent_node_ = nullptr;
  node->previous_sibling_ = nullptr;
}

template <class NodeClass>
void Node<NodeClass>::ReplaceChild(NodeClass* new_node, NodeClass* old_node) {
  DCHECK_NE(new_node, old_node);
  DCHECK_EQ(old_node->parent_node_, this);
  if (auto const parent = new_node->parent_node_)
    parent->RemoveChild(new_node);
  auto const previous = old_node->previous_sibling_;
  if (previous) {
    previous->next_sibling_ = new_node;
  } else {
    DCHECK_EQ(first_child_, old_node);
    first_child_ = new_node;
  }
  auto const next = old_node->next_sibling_;
  if (next) {
    next->previous_sibling_ = new_node;
  } else {
    DCHECK_EQ(last_child_, old_node);
    last_child_ = new_node;
  }
  old_node->next_sibling_ = nullptr;
  old_node->parent_node_ = nullptr;
  old_node->previous_sibling_ = nullptr;
  new_node->next_sibling_ = next;
  new_node->previous_sibling_ = previous;
  new_node->parent_node_ = static_cast<NodeClass*>(this);
}

}  // namespace tree
}  // namespace common

#endif  // COMMON_TREE_NODE_H_

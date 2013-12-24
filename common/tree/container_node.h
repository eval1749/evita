// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_container_node_h)
#define INCLUDE_common_tree_container_node_h

#include "common/tree/abstract_node_iterator.h"
#include "common/tree/node.h"

namespace common {
namespace tree {

template<typename ContainerType> class ChildNodes_;

template<class NodeClass, class ContainerClass, typename... Params>
class ContainerNode_ : public NodeClass {
  public: typedef ContainerClass Container;
  public: typedef NodeClass Node;
  public: typedef common::tree::ChildNodes_<const ContainerNode_>
      ConstChildNodes;
  public: typedef common::tree::ChildNodes_<ContainerNode_> ChildNodes;

  private: NodeClass* first_child_;
  private: NodeClass* last_child_;

  protected: ContainerNode_(Params... params)
    : NodeClass(std::move(params...)),
      first_child_(nullptr),
      last_child_(nullptr) {
  }

  public: ChildNodes child_nodes() { return ChildNodes(*this); }

  public: ConstChildNodes child_nodes() const {
    return ConstChildNodes(*this);
  }

  public: virtual NodeClass* first_child() const override {
    return first_child_;
  }
  public: virtual NodeClass* last_child() const override {
    return last_child_;
  }

  public: void AppendChild(NodeClass& node) {
    if (auto const old_parent = node.parent_node_)
      old_parent->RemoveChild(node);
    ASSERT(!node.parent_node());
    ASSERT(!node.next_sibling());
    ASSERT(!node.previous_sibling());
    node.previous_sibling_ = last_child_;
    if (last_child_) {
      ASSERT(first_child_);
      last_child_->next_sibling_ = &node;
    } else {
      ASSERT(!first_child_);
      first_child_ = &node;
    }
    last_child_ = &node;
    node.parent_node_ = static_cast<ContainerClass*>(this);
  }

  public: virtual bool Contains(const NodeClass& node) const override;

  public: void InsertAfter(NodeClass& node, NodeClass* ref_node) {
    if (!ref_type) {
      PreprendChild(node);
      return;
    }
    ASSERT(node != ref_node);
    ASSERT(ref_node->parent_node == this);
    if (auto const old_parent = node.parent_node_)
      old_parent->RemoveChild(node);
    auto const next = ref_node->next;
    if (next) {
      next->previous_sibling_ = &node;
    } else {
      ASSERT(last_child_ == ref_node);
      last_child_ = &node;
    }
    ref_node->next_sibling_= &node;
    node->next_sibling = next;
    node->previous_sibling_ = ref_node;
    node.parent_node_ = static_cast<ContainerClass*>(this);
  }

  public: void InsertBefore(NodeClass& node, NodeClass* ref_node) {
    if (!ref_node) {
      AppendChild(node);
      return;
    }
    ASSERT(node != ref_node);
    ASSERT(ref_node->parent_node_ == this);
    if (auto const old_parent = node.parent_node_)
      old_parent->RemoveChild(node);
    auto const previous = ref_node->previous_sibling_;
    if (previous) {
      previous->next_sibling_ = &node;
    } else {
      ASSERT(first_child_ == ref_node);
      first_child_ = &node;
    }
    ref_node->previous_sibling_ = &node;
    node.next_sibling_ = ref_node;
    node.previous_sibling_ = previous;
    node.parent_node_ = static_cast<ContainerClass*>(this);
  }

  public: void PrependChild(NodeClass& node) {
    if (auto const old_parent = node.parent_node_)
      old_parent->RemoveChild(node);
    ASSERT(!node.parent_node());
    ASSERT(!node.next_sibling())
    ASSERT(!node.previous_sibling())
    node.next_sibling_ = first_child_;
    if (first_child_) {
      ASSERT(last_child_);
      first_child_->previous_sibling_ = &node;
    } else {
      ASSERT(!last_child_);
      last_child_ = &node;
    }
    first_child_ = &node;
    node.parent_node_ = static_cast<ContainerClass*>(this);
  }

  public: void RemoveChild(NodeClass& node) {
    ASSERT(node.parent_node_ == this);
    auto const previous = node.previous_sibling();
    auto const next = node.next_sibling();
    if (previous) {
      previous->next_sibling_ = next;
    } else {
      ASSERT(first_child_ == &node);
      first_child_ = next;
    }
    if (next) {
      next->previous_sibling_ = previous;
    } else {
      ASSERT(last_child_ == &node);
      last_child_ = previous;
    }
    node.next_sibling_ = nullptr;
    node.parent_node_ = nullptr;
    node.previous_sibling_ = nullptr;
  }

  DISALLOW_COPY_AND_ASSIGN(ContainerNode_);
};
} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_container_node_h)

// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_tree_node_h)
#define INCLUDE_base_tree_node_h

namespace base {
namespace tree {

template<class NodeClass, class ContainerClass, typename... Params>
class ContainerNode_;

template<class NodeClass, class ContainerClass, typename... Params>
class Node_ {
  friend class ContainerNode_<NodeClass, ContainerClass, Params...>;
  public: typedef ContainerClass Container;
  public: typedef NodeClass Node;
  private: NodeClass* next_sibling_;
  private: ContainerClass* parent_node_;
  private: NodeClass* previous_sibling_;

  public: Node_()
    : next_sibling_(nullptr),
      parent_node_(nullptr),
      previous_sibling_(nullptr) {
  }
  public: virtual ~Node_() {
  }

  public: bool operator==(const NodeClass& other) const {
    return this == &other;
  }

  public: bool operator==(const NodeClass* other) const {
    return this == other;
  }

  public: bool operator!=(const NodeClass& other) const {
    return this != &other;
  }

  public: bool operator!=(const NodeClass* other) const {
    return this != other;
  }

  public: virtual NodeClass* first_child() const { return nullptr; }
  public: virtual NodeClass* last_child() const { return nullptr; }
  public: NodeClass* next_sibling() const { return next_sibling_; }
  public: ContainerClass* parent_node() const { return parent_node_; }
  public: NodeClass* previous_sibling() const { return previous_sibling_; }

  public: virtual bool Contains(const NodeClass&) const { return false; }

  DISALLOW_COPY_AND_ASSIGN(Node_);
};
} // namespace tree
} // namespace base

#endif //!defined(INCLUDE_base_tree_node_h)

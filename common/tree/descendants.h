// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_tree_descendants_h)
#define INCLUDE_common_tree_descendants_h

#include "common/tree/descendants_or_self.h"

namespace common {
namespace tree {
namespace impl {

template<typename NodeType>
class Descendants_ : public DescendantsOrSelf_<NodeType> {
  public: explicit Descendants_(RefType container) 
      : DescendantsOrSelf_(container) {
  }

  public: Iterator begin() const {
    return Iterator(&node_, node_.first_child());
  }
  public: Iterator end() const { return Iterator(&node_, nullptr);
  }

  DISALLOW_COPY_AND_ASSIGN(Descendants_);
};

} // naemspace impl

template<class ContainerClass>
impl::Descendants_<const typename ContainerClass::Node>
descendants(const ContainerClass& container) {
  return impl::Descendants_<const ContainerClass::Node>(container);
}

template<class ContainerClass>
impl::Descendants_<typename ContainerClass::Node>
descendants(ContainerClass& container) {
  return impl::Descendants_<ContainerClass::Node>(container);
}

template<class NodeClass, class ContainerClass, typename... Params>
bool ContainerNode_<NodeClass, ContainerClass, Params...>::Contains(
    const NodeClass& node) const {
  for (const auto& child: descendants(*this)) {
    if (child == node)
      return true;
  }
  return false;
}

} // namespace tree
} // namespace common

#endif //!defined(INCLUDE_common_tree_descendants_h)

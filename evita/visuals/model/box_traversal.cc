// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_traversal.h"

#include "evita/visuals/model/ancestors.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTraversal
//

Box* BoxTraversal::FirstChildOf(const Box& box) {
  const auto container = box.as<ContainerBox>();
  if (!container)
    return nullptr;
  return container->first_child();
}

Box* BoxTraversal::LastChildOf(const Box& box) {
  const auto container = box.as<ContainerBox>();
  if (!container)
    return nullptr;
  return container->last_child();
}

Box* BoxTraversal::NextOf(const Box& box) {
  if (const auto first_child = FirstChildOf(box))
    return first_child;
  if (const auto next_sibling = NextSiblingOf(box))
    return next_sibling;
  for (const auto& runner : Box::Ancestors(box)) {
    if (const auto next = NextSiblingOf(*runner))
      return next;
  }
  return nullptr;
}

Box* BoxTraversal::NextSiblingOf(const Box& box) {
  return box.next_sibling();
}

ContainerBox* BoxTraversal::ParentOf(const Box& box) {
  return box.parent();
}

Box* BoxTraversal::PreviousSiblingOf(const Box& box) {
  return box.previous_sibling();
}

}  // namespace visuals

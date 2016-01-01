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
  if (container->child_boxes().empty())
    return nullptr;
  return container->child_boxes().front();
}

Box* BoxTraversal::LastChildOf(const Box& box) {
  const auto container = box.as<ContainerBox>();
  if (!container)
    return nullptr;
  if (container->child_boxes().empty())
    return nullptr;
  return container->child_boxes().back();
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

// TODO(eval1749): We should have next/previous sibling link for |Box|.
Box* BoxTraversal::NextSiblingOf(const Box& box) {
  auto found = false;
  const auto parent = ParentOf(box);
  if (!parent)
    return nullptr;
  for (const auto& child : parent->child_boxes()) {
    if (found)
      return child;
    found = &box == child;
  }
  return nullptr;
}

ContainerBox* BoxTraversal::ParentOf(const Box& box) {
  return box.parent();
}

}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/container_box.h"

#include "base/logging.h"
#include "evita/visuals/style/margin.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
ContainerBox::ContainerBox() {}

ContainerBox::~ContainerBox() {
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), parent());
  for (const auto& child : child_boxes_) {
    Box::Editor(child).SetParent(nullptr);
    delete child;
  }
}

Box* ContainerBox::AppendChild(std::unique_ptr<Box> child) {
  DCHECK_NE(this, child.get());
  DCHECK(!child->IsDescendantOf(*this));
  DCHECK(!IsDescendantOf(*child));
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), child->parent());
  Box::Editor(child.get()).SetParent(this);
  child_boxes_.push_back(child.release());
  return child_boxes_.back();
}

std::unique_ptr<Box> ContainerBox::RemoveChild(Box* box) {
  DCHECK_EQ(this, box->parent());
  const auto it = std::find(child_boxes_.begin(), child_boxes_.end(), box);
  DCHECK(it != child_boxes_.end());
  Box::Editor(box).SetParent(nullptr);
  child_boxes_.erase(it);
  return std::unique_ptr<Box>(box);
}

}  // namespace visuals

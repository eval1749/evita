// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/container_box.h"

#include "base/logging.h"
#include "evita/visuals/model/box_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//

ContainerBox::ContainerBox(RootBox* root_box, const base::StringPiece16& id)
    : Box(root_box, id) {}

ContainerBox::ContainerBox(RootBox* root_box) : Box(root_box) {}

ContainerBox::~ContainerBox() {
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), parent());
  for (const auto& child : child_boxes_) {
    BoxEditor().WillDestroy(child);
    delete child;
  }
}

Box* ContainerBox::first_child() const {
  if (child_boxes_.empty())
    return nullptr;
  return child_boxes_.front();
}

Box* ContainerBox::last_child() const {
  if (child_boxes_.empty())
    return nullptr;
  return child_boxes_.back();
}

}  // namespace visuals

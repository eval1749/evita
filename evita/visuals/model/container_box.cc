// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/container_box.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
ContainerBox::ContainerBox(Display display) : Box(display) {}

ContainerBox::~ContainerBox() {}

void ContainerBox::AppendChild(Box* box) {
  DCHECK_NE(this, box);
  DCHECK(!box->IsDescendantOf(*this));
  DCHECK(!IsDescendantOf(*box));
  if (auto old_parent = box->parent())
    old_parent->RemoveChild(box);
  Editor(box).SetParent(this);
  child_boxes_.push_back(box);
}

void ContainerBox::RemoveChild(Box* box) {
  DCHECK_EQ(this, box->parent());
  const auto it = std::find(child_boxes_.begin(), child_boxes_.end(), box);
  DCHECK(it != child_boxes_.end());
  Editor(box).SetParent(nullptr);
  child_boxes_.erase(it);
}

void ContainerBox::SetContentBounds(const FloatRect& new_bounds) {
  if (content_bounds_ == new_bounds)
    return;
  content_bounds_ = new_bounds;
  DidChangeContent();
}

}  // namespace visuals

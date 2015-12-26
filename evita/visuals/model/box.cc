// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"
#include "evita/visuals/style/display.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box
//
Box::Box() {}
Box::~Box() {}

void Box::DidChangeContent() {
  is_content_dirty_ = true;
  if (!parent_ || parent_->is_content_dirty_)
    return;
  parent_->DidChangeContent();
}

void Box::DidChangeLayout() {
  is_layout_dirty_ = true;
  if (!parent_ || parent_->is_layout_dirty_)
    return;
  parent_->DidChangeLayout();
}

bool Box::IsDescendantOf(const Box& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// Box::Editor
//
Box::Editor::Editor(Box* box) : box_(box) {}
Box::Editor::~Editor() {}

void Box::Editor::SetBounds(const FloatRect& new_bounds) {
  if (box_->bounds_ == new_bounds)
    return;
  box_->bounds_ = new_bounds;
  box_->content_bounds_ = FloatRect(
      FloatPoint() + box_->border().top_left() + box_->padding().top_left(),
      new_bounds.size() - box_->border().top_left() -
          box_->padding().top_left() - box_->border().bottom_right() -
          box_->padding().bottom_right());
}

void Box::Editor::SetLayoutClean() {
  DCHECK(box_->is_layout_dirty_);
  box_->is_layout_dirty_ = false;
}

void Box::Editor::SetParent(ContainerBox* new_parent) {
  DCHECK_NE(box_->parent_, new_parent);
  box_->parent_ = new_parent;
}

std::ostream& operator<<(std::ostream& ostream, const Box& box) {
  return ostream << box.class_name() << box.bounds();
}

std::ostream& operator<<(std::ostream& ostream, const Box* box) {
  if (!box)
    return ostream << "nullptr";
  return ostream << *box;
}

}  // namespace visuals

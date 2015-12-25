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
Box::Box(Display display) : display_(display) {}
Box::~Box() {}

void Box::DidChangeContent() {
  dirty_ = true;
}

bool Box::IsBlock() const {
  return display_ == Display::Block || display_ == Display::InlineBlock;
}

bool Box::IsDescendantOf(const Box& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

bool Box::IsInline() const {
  return display_ == Display::Inline || display_ == Display::InlineBlock;
}

void Box::SetBounds(const FloatRect& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  dirty_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// Box::Editor
//
Box::Editor::Editor(Box* box) : box_(box) {}
Box::Editor::~Editor() {}

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

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_editor.h"

#include "base/logging.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/style/border.h"
#include "evita/visuals/style/margin.h"
#include "evita/visuals/style/padding.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxEditor
//
BoxEditor::BoxEditor() {}
BoxEditor::~BoxEditor() {}

Box* BoxEditor::AppendChild(ContainerBox* container,
                            std::unique_ptr<Box> new_child) {
  DCHECK_NE(container, new_child.get());
  DCHECK(!new_child->IsDescendantOf(*container));
  DCHECK(!container->IsDescendantOf(*new_child));
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->parent());
  new_child->parent_ = container;
  container->child_boxes_.push_back(new_child.release());
  DidChangeContent(container);
  // TODO(eval1749): If |container| has fixed width/height, layout of
  // container of |container| isn't changed.
  DidChangeLayout(container);
  return container->child_boxes_.back();
}

void BoxEditor::DidChangeContent(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelfOf(*box)) {
    if (runner->is_content_dirty_)
      return;
    runner->is_content_dirty_ = true;
  }
}

void BoxEditor::DidChangeLayout(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelfOf(*box)) {
    if (runner->is_layout_dirty_)
      return;
    runner->is_layout_dirty_ = true;
  }
}

std::unique_ptr<Box> BoxEditor::RemoveChild(ContainerBox* container,
                                            Box* old_child) {
  DCHECK_EQ(container, old_child->parent());
  const auto it = std::find(container->child_boxes_.begin(),
                            container->child_boxes_.end(), old_child);
  DCHECK(it != container->child_boxes_.end());
  container->child_boxes_.erase(it);
  old_child->parent_ = nullptr;
  DidChangeContent(container);
  // TODO(eval1749): If |container| has fixed width/height, layout of
  // container of |container| isn't changed.
  DidChangeLayout(container);
  return std::unique_ptr<Box>(old_child);
}

void BoxEditor::SetBaseline(TextBox* box, float new_baseline) {
  if (box->baseline_ == new_baseline)
    return;
  box->baseline_ = new_baseline;
  DidChangeContent(box);
}

void BoxEditor::SetColor(TextBox* box, const FloatColor& new_color) {
  if (box->color_ == new_color)
    return;
  box->color_ = new_color;
  DidChangeContent(box);
}

void BoxEditor::SetBounds(Box* box, const FloatRect& new_bounds) {
  if (box->bounds_ == new_bounds)
    return;
  box->bounds_ = new_bounds;
  box->content_bounds_ = FloatRect(
      FloatPoint() + box->border().top_left() + box->padding().top_left(),
      new_bounds.size() - box->border().top_left() - box->padding().top_left() -
          box->border().bottom_right() - box->padding().bottom_right());
}

void BoxEditor::SetLayoutClean(Box* box) {
  DCHECK(box->is_layout_dirty_);
  box->is_layout_dirty_ = false;
}

void BoxEditor::SetParent(Box* box, ContainerBox* new_parent) {
  DCHECK_NE(box->parent_, new_parent);
  box->parent_ = new_parent;
  DidChangeLayout(box);
}

}  // namespace visuals

// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_editor.h"

#include "base/logging.h"
#include "evita/visuals/model/ancestors.h"
#include "evita/visuals/model/ancestors_or_self.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/style/style.h"

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
  if (box->is_content_dirty_)
    return;
  box->is_content_dirty_ = true;
  for (const auto& runner : Box::Ancestors(*box)) {
    if (runner->is_content_dirty_ || runner->is_child_content_dirty_)
      return;
    runner->is_child_content_dirty_ = true;
  }
}

void BoxEditor::DidChangeLayout(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelf(*box)) {
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

void BoxEditor::SetBounds(Box* box, const FloatRect& new_bounds) {
  if (box->bounds_ == new_bounds)
    return;
  box->bounds_ = new_bounds;
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

void BoxEditor::SetStyle(Box* box, const Style& new_style) {
  auto is_content_dirty = false;
  auto is_layout_dirty = false;
  if (new_style.has_background() &&
      new_style.background() != box->background_) {
    box->background_ = new_style.background();
    is_content_dirty = true;
  }
  if (new_style.has_border() && new_style.border() != box->border_) {
    box->border_ = new_style.border();
    is_content_dirty = true;
  }
  if (new_style.has_padding() && new_style.padding() != box->padding_) {
    box->padding_ = new_style.padding();
    is_content_dirty = true;
  }
  if (new_style.has_margin() && new_style.margin() != box->margin_) {
    box->margin_ = new_style.margin();
    is_layout_dirty = true;
  }

  if (const auto& text = box->as<TextBox>()) {
    if (new_style.has_color() && new_style.color() != text->color_) {
      text->color_ = new_style.color();
      is_content_dirty = true;
    }
  }

  if (is_content_dirty)
    DidChangeContent(box);
  if (is_layout_dirty)
    DidChangeLayout(box);
}

}  // namespace visuals

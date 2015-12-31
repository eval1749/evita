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
#include "evita/visuals/css/style.h"

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
  box->is_layout_dirty_ = false;
}

void BoxEditor::SetParent(Box* box, ContainerBox* new_parent) {
  DCHECK_NE(box->parent_, new_parent);
  box->parent_ = new_parent;
  DidChangeLayout(box);
}

// When following one of following property is changed, we need to layout
// contained boxes in this box and need to paint border and content.
//  border: size of content area is changed.
//  padding: size of content area is changed.
// TODO(eval1749): When only border color changed, we should not layout and
// paint content.
#define FOR_EACH_PROPERTY_CAUSING_SELF_LAYOUT(V) \
  V(border)                                      \
  V(padding)

// When following one of following property is changed, we need to layout
// contained boxes in parent.
//  bottom, left, right, top: Asks containing block to calculate position of
//    this box.
//  margin: When position == static, following position of following sibling
//    boxes are dirty.
//    When position != static, a position of this box is dirty.
//  position: Position of following boxes are dirty, when position is from/to
//    static, or a position of this box, when position is not from/to static.
#define FOR_EACH_PROPERTY_CAUSING_PARENT_LAYOUT(V) \
  V(bottom)                                        \
  V(left)                                          \
  V(margin)                                        \
  V(position)                                      \
  V(right)                                         \
  V(top)

void BoxEditor::SetStyle(Box* box, const css::Style& new_style) {
  auto is_content_dirty = false;
  auto is_layout_dirty = false;
  auto is_parent_layout_dirty = false;

  if (new_style.has_background() &&
      new_style.background() != box->background_) {
    box->background_ = new_style.background();
    is_content_dirty = true;
  }

  if (new_style.has_display() &&
      new_style.display().is_none() != box->is_display_none_) {
    box->is_display_none_ = new_style.display().is_none();
    is_parent_layout_dirty = true;
  }

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    is_content_dirty = true;                      \
    is_layout_dirty = true;                       \
  }
  FOR_EACH_PROPERTY_CAUSING_SELF_LAYOUT(V)
#undef V

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    is_parent_layout_dirty = true;                \
  }
  FOR_EACH_PROPERTY_CAUSING_PARENT_LAYOUT(V)
#undef V

  if (const auto& text = box->as<TextBox>()) {
    if (new_style.has_color() && new_style.color().value() != text->color_) {
      text->color_ = new_style.color().value();
      is_content_dirty = true;
    }
  }

  if (is_content_dirty)
    DidChangeContent(box);
  if (is_layout_dirty)
    DidChangeLayout(box);
  if (is_parent_layout_dirty && box->parent_)
    DidChangeLayout(box->parent_);
}

}  // namespace visuals

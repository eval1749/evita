// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_editor.h"

#include "base/logging.h"
#include "evita/visuals/layout/ancestors.h"
#include "evita/visuals/layout/ancestors_or_self.h"
#include "evita/visuals/layout/block_flow_box.h"
#include "evita/visuals/layout/descendants_or_self.h"
#include "evita/visuals/layout/inline_flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/css/style.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxEditor
//
BoxEditor::BoxEditor() {}
BoxEditor::~BoxEditor() {}

void BoxEditor::AppendChild(ContainerBox* container, Box* new_child) {
  DCHECK_NE(container, new_child);
  DCHECK(!new_child->IsDescendantOf(*container));
  DCHECK(!container->IsDescendantOf(*new_child));
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->parent_);
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->next_sibling_);
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), new_child->previous_sibling_);
  new_child->parent_ = container;
  if (const auto old_last_child = container->last_child_) {
    new_child->previous_sibling_ = old_last_child;
    old_last_child->next_sibling_ = new_child;
  } else {
    container->first_child_ = new_child;
  }
  container->last_child_ = new_child;
  MarkDirty(container);
  new_child->version_ = container->version_;
}

void BoxEditor::DidLayout(Box* box) {
  // TODO(eval1749): What should we do here?
}

void BoxEditor::DidMove(Box* box) {
  box->is_origin_changed_ = true;
  SetShouldPaint(box);
}

void BoxEditor::DidPaint(Box* box) {
  DCHECK(box->root_box()->InPaint());
  box->is_background_changed_ = false;
  box->is_border_changed_ = false;
  box->is_content_changed_ = false;
  box->is_origin_changed_ = false;
  box->is_padding_changed_ = false;
  box->is_size_changed_ = false;
  box->should_paint_ = false;

  box->is_changed_ = false;
  const auto container = box->as<ContainerBox>();
  if (!container)
    return;
  container->is_child_changed_ = false;
}

void BoxEditor::MarkDirty(Box* box) {
  ++box->root_box_->version_;
  box->version_ = box->root_box_->version_;
  box->is_changed_ = true;
  ScheduleVisualUpdateIfNeeded(box);
  for (const auto& runner : Box::Ancestors(*box)) {
    if (runner->is_changed_ || runner->is_child_changed_)
      return;
    runner->is_child_changed_ = true;
  }
}

void BoxEditor::RemoveAllChildren(ContainerBox* container) {
  while (const auto child = container->first_child()) {
    RemoveChild(container, child);
    if (!child->node()) {
      // Delete anonymous box.
      delete child;
    }
  }
}

void BoxEditor::RemoveChild(ContainerBox* container, Box* old_child) {
  DCHECK_EQ(container, old_child->parent_);
  const auto next_sibling = old_child->next_sibling_;
  const auto previous_sibling = old_child->previous_sibling_;

  if (next_sibling)
    next_sibling->previous_sibling_ = old_child->previous_sibling_;
  else
    container->last_child_ = old_child->previous_sibling_;
  if (previous_sibling)
    previous_sibling->next_sibling_ = next_sibling;
  else
    container->first_child_ = next_sibling;

  old_child->next_sibling_ = nullptr;
  old_child->previous_sibling_ = nullptr;
  old_child->parent_ = nullptr;
  MarkDirty(container);
}

void BoxEditor::RemoveDescendants(ContainerBox* container_box) {
  while (auto const child = container_box->first_child()) {
    if (const auto child_container_box = child->as<ContainerBox>())
      RemoveDescendants(child_container_box);
    RemoveChild(container_box, child);
  }
}

void BoxEditor::ScheduleVisualUpdateIfNeeded(Box* box) {
  if (!box->InDocument())
    return;
  box->root_box()->lifecycle()->Reset();
}

void BoxEditor::SetBaseline(TextBox* box, float new_baseline) {
  if (box->baseline_ == new_baseline)
    return;
  box->baseline_ = new_baseline;
  box->is_content_changed_ = true;
  MarkDirty(box);
  ScheduleVisualUpdateIfNeeded(box);
}

void BoxEditor::SetBounds(Box* box, const FloatRect& new_bounds) {
  DCHECK(box->root_box()->InLayout());
  if (box->bounds_ == new_bounds)
    return;
  if (box->bounds_.origin() != new_bounds.origin())
    box->is_origin_changed_ = true;
  if (box->bounds_.size() != new_bounds.size()) {
    if (box->background().HasValue())
      box->is_background_changed_ = true;
    if (box->border().HasValue())
      box->is_border_changed_ = true;
    box->is_size_changed_ = true;
  }
  box->bounds_ = new_bounds;
}

void BoxEditor::SetContentChanged(ContentBox* box) {
  box->is_content_changed_ = true;
  MarkDirty(box);
  ScheduleVisualUpdateIfNeeded(box);
}

#define FOR_EACH_PROPERTY_CHANGES_PROPERTY(V) \
  V(background)                               \
  V(border)                                   \
  V(padding)

#define FOR_EACH_PROPERTY_AFFECTS_ORIGIN(V) \
  V(bottom)                                 \
  V(left)                                   \
  V(margin)                                 \
  V(position)                               \
  V(right)                                  \
  V(top)

#define FOR_EACH_PROPERTY_AFFECTS_SIZE(V) \
  V(height)                               \
  V(width)

void BoxEditor::SetStyle(Box* box, const css::Style& new_style) {
  auto is_changed = false;

  if (const auto& text = box->as<TextBox>()) {
    // |TextBox| uses only color, ant font related CSS properties.
    if (new_style.has_color() && new_style.color().value() != text->color_) {
      text->color_ = new_style.color().value();
      box->is_content_changed_ = true;
      MarkDirty(text);
    }
    return;
  }

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_##property##_changed_ = true;         \
    is_changed = true;                            \
  }
  FOR_EACH_PROPERTY_CHANGES_PROPERTY(V)
#undef V

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_origin_changed_ = true;               \
    is_changed = true;                            \
  }
  FOR_EACH_PROPERTY_AFFECTS_ORIGIN(V)
#undef V

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_size_changed_ = true;                 \
    is_changed = true;                            \
  }
  FOR_EACH_PROPERTY_AFFECTS_SIZE(V)
#undef V

  if (!is_changed)
    return;
  MarkDirty(box);
}

void BoxEditor::SetShouldPaint(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelf(*box)) {
    if (runner->should_paint_)
      return;
    runner->should_paint_ = true;
  }
}

void BoxEditor::SetTextColor(TextBox* text_box, const FloatColor& color) {
  if (text_box->color_ == color)
    return;
  text_box->color_ = color;
  SetContentChanged(text_box);
}

void BoxEditor::SetViewportSize(RootBox* root_box, const FloatSize& size) {
  DCHECK(root_box->lifecycle_.AllowsTreeMutaions()) << root_box->lifecycle_;
  if (root_box->viewport_size_ == size)
    return;
  root_box->bounds_ = FloatRect(size);
  // TODO(eval1749): We don't need to have |RootBox::viewport_size_|. We can
  // use |RootBox::bounds_.size()|.
  root_box->viewport_size_ = size;
  root_box->is_size_changed_ = true;
  ScheduleVisualUpdateIfNeeded(root_box);
}

void BoxEditor::WillDestroy(Box* box) {
  box->parent_ = nullptr;
}

}  // namespace visuals

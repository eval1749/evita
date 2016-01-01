// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_editor.h"

#include "base/logging.h"
#include "evita/visuals/model/ancestors.h"
#include "evita/visuals/model/ancestors_or_self.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/descendants_or_self.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/css/style.h"

namespace visuals {

namespace {

RootBox* FindRootBox(const Box& box) {
  for (const auto& runner : Box::AncestorsOrSelf(box)) {
    if (const auto root_box = runner->as<RootBox>())
      return root_box;
  }
  return nullptr;
}
}  // namespace

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
  if (const auto root_box = FindRootBox(*container)) {
    for (const auto runner : Box::DescendantsOrSelf(*new_child))
      root_box->RegisterBoxIdIfNeeded(*runner);
  }
  container->child_boxes_.push_back(new_child.release());
  DidChangeChild(container->parent());
  return container->child_boxes_.back();
}

void BoxEditor::DidChangeChild(ContainerBox* container) {
  ScheduleVisualUpdateIfNeeded(container);
  if (!container || container->is_children_changed_)
    return;
  container->is_children_changed_ = true;
  for (const auto& runner : Box::Ancestors(*container)) {
    if (runner->is_subtree_changed_)
      return;
    runner->is_subtree_changed_ = true;
  }
}

void BoxEditor::DidLayout(Box* box) {
  // TODO(eval1749): What should we do here?
}

void BoxEditor::DidMove(Box* box) {
  box->is_origin_changed_ = true;
  SetShouldPaint(box);
}

// TODO(eval1749): We should make sure |BoxEditor::DidPaint()| is called during
// painting.
void BoxEditor::DidPaint(Box* box) {
  box->is_background_changed_ = false;
  box->is_border_changed_ = false;
  box->is_content_changed_ = false;
  box->is_origin_changed_ = false;
  box->is_padding_changed_ = false;
  box->is_size_changed_ = false;
  box->should_paint_ = false;
  const auto container = box->as<ContainerBox>();
  if (!container)
    return;
  container->is_children_changed_ = false;
  container->is_subtree_changed_ = false;
}

std::unique_ptr<Box> BoxEditor::RemoveChild(ContainerBox* container,
                                            Box* old_child) {
  DCHECK_EQ(container, old_child->parent());
  const auto it = std::find(container->child_boxes_.begin(),
                            container->child_boxes_.end(), old_child);
  DCHECK(it != container->child_boxes_.end());
  container->child_boxes_.erase(it);
  if (const auto root_box = FindRootBox(*container)) {
    for (const auto runner : Box::DescendantsOrSelf(*old_child))
      root_box->UnregisterBoxIdIfNeeded(*runner);
  }
  old_child->parent_ = nullptr;
  DidChangeChild(container);
  return std::unique_ptr<Box>(old_child);
}

void BoxEditor::ScheduleVisualUpdateIfNeeded(Box* box) {
  const auto root_box = FindRootBox(*box);
  if (!root_box)
    return;
  root_box->lifecycle()->Reset();
}

void BoxEditor::SetBaseline(TextBox* box, float new_baseline) {
  if (box->baseline_ == new_baseline)
    return;
  box->baseline_ = new_baseline;
  box->is_content_changed_ = true;
  ScheduleVisualUpdateIfNeeded(box);
}

void BoxEditor::SetBounds(Box* box, const FloatRect& new_bounds) {
  // TODO(eval1749): We should make |Box| always have |RootBox|.
  DCHECK(FindRootBox(*box)->InLayout());
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

void BoxEditor::SetContentChanged(InlineBox* box) {
  box->is_content_changed_ = true;
  ScheduleVisualUpdateIfNeeded(box);
}

#define FOR_EACH_PROPERTY_CHANGES_PROPERTY(V) \
  V(background)                               \
  V(border)                                   \
  V(padding)

#define FOR_EACH_PROPERTY_CHANGES_ORIGIN(V) \
  V(bottom)                                 \
  V(left)                                   \
  V(margin)                                 \
  V(position)                               \
  V(right)                                  \
  V(top)

void BoxEditor::SetStyle(Box* box, const css::Style& new_style) {
  if (new_style.has_display() &&
      new_style.display().is_none() != box->is_display_none_) {
    box->is_display_none_ = new_style.display().is_none();
    DidChangeChild(box->parent_);
  }

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_##property##_changed_ = true;         \
  }
  FOR_EACH_PROPERTY_CHANGES_PROPERTY(V)
#undef V

#define V(property)                               \
  if (new_style.has_##property() &&               \
      new_style.property() != box->property##_) { \
    box->property##_ = new_style.property();      \
    box->is_origin_changed_ = true;               \
  }
  FOR_EACH_PROPERTY_CHANGES_ORIGIN(V)
#undef V

  if (const auto& text = box->as<TextBox>()) {
    if (new_style.has_color() && new_style.color().value() != text->color_) {
      text->color_ = new_style.color().value();
      SetContentChanged(text);
    }
  }

  if (!box->parent_)
    return;
  if (!box->is_origin_changed_ && !box->is_size_changed_)
    return;

  // Since changing size or origin affects parent's size and sibling's origin,
  // we should notify to parent.
  DidChangeChild(box->parent_);
}

void BoxEditor::SetShouldPaint(Box* box) {
  for (const auto& runner : Box::AncestorsOrSelf(*box)) {
    if (runner->should_paint_)
      return;
    runner->should_paint_ = true;
  }
}

void BoxEditor::SetViewportSize(RootBox* root_box, const FloatSize& size) {
  DCHECK(root_box->lifecycle_.AllowsTreeMutaions()) << root_box->lifecycle_;
  if (root_box->viewport_size_ == size)
    return;
  root_box->viewport_size_ = size;
  root_box->is_size_changed_ = true;
  ScheduleVisualUpdateIfNeeded(root_box);
}

void BoxEditor::WillDestroy(Box* box) {
  box->parent_ = nullptr;
}

}  // namespace visuals

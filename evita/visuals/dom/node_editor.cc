// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_editor.h"

#include "base/logging.h"
#include "evita/visuals/dom/ancestors.h"
#include "evita/visuals/dom/ancestors_or_self.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/text_node.h"
#include "evita/visuals/css/style.h"

namespace visuals {

namespace {

Document* FindDocument(const Node& node) {
  for (const auto& runner : Node::AncestorsOrSelf(node)) {
    if (const auto document = runner->as<Document>())
      return document;
  }
  return nullptr;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NodeEditor
//
NodeEditor::NodeEditor() {}
NodeEditor::~NodeEditor() {}

Node* NodeEditor::AppendChild(ContainerNode* container,
                              std::unique_ptr<Node> new_child_ptr) {
  const auto new_child = new_child_ptr.release();
  DCHECK_NE(container, new_child);
  DCHECK(!new_child->IsDescendantOf(*container));
  DCHECK(!container->IsDescendantOf(*new_child));
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->parent_);
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->next_sibling_);
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->previous_sibling_);
  new_child->parent_ = container;
  if (const auto old_last_child = container->last_child_) {
    new_child->previous_sibling_ = old_last_child;
    old_last_child->next_sibling_ = new_child;
  } else {
    container->first_child_ = new_child;
  }
  container->last_child_ = new_child;
  if (const auto document = FindDocument(*container)) {
    for (const auto runner : Node::DescendantsOrSelf(*new_child))
      document->RegisterNodeIdIfNeeded(*runner);
  }
  DidChangeChild(container->parent_);
  return new_child;
}

void NodeEditor::DidChangeChild(ContainerNode* container) {
  ScheduleVisualUpdateIfNeeded(container);
  if (!container || container->is_children_changed_)
    return;
  container->is_children_changed_ = true;
  for (const auto& runner : Node::Ancestors(*container)) {
    if (runner->is_subtree_changed_)
      return;
    runner->is_subtree_changed_ = true;
  }
}

void NodeEditor::DidLayout(Node* node) {
  // TODO(eval1749): What should we do here?
}

void NodeEditor::DidMove(Node* node) {
  node->is_origin_changed_ = true;
  SetShouldPaint(node);
}

void NodeEditor::DidPaint(Node* node) {
  DCHECK(node->document()->InPaint());
  node->is_background_changed_ = false;
  node->is_border_changed_ = false;
  node->is_content_changed_ = false;
  node->is_origin_changed_ = false;
  node->is_padding_changed_ = false;
  node->is_size_changed_ = false;
  node->should_paint_ = false;
  const auto container = node->as<ContainerNode>();
  if (!container)
    return;
  container->is_children_changed_ = false;
  container->is_subtree_changed_ = false;
}

std::unique_ptr<Node> NodeEditor::RemoveChild(ContainerNode* container,
                                              Node* old_child) {
  DCHECK_EQ(container, old_child->parent_);
  if (const auto document = FindDocument(*container)) {
    for (const auto runner : Node::DescendantsOrSelf(*old_child))
      document->UnregisterNodeIdIfNeeded(*runner);
  }

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
  DidChangeChild(container);
  return std::unique_ptr<Node>(old_child);
}

void NodeEditor::ScheduleVisualUpdateIfNeeded(Node* node) {
  const auto document = FindDocument(*node);
  if (!document)
    return;
  document->lifecycle()->Reset();
}

void NodeEditor::SetBaseline(TextNode* node, float new_baseline) {
  if (node->baseline_ == new_baseline)
    return;
  node->baseline_ = new_baseline;
  node->is_content_changed_ = true;
  ScheduleVisualUpdateIfNeeded(node);
}

void NodeEditor::SetBounds(Node* node, const FloatRect& new_bounds) {
  DCHECK(node->document()->InLayout());
  if (node->bounds_ == new_bounds)
    return;
  if (node->bounds_.origin() != new_bounds.origin())
    node->is_origin_changed_ = true;
  if (node->bounds_.size() != new_bounds.size()) {
    if (node->background().HasValue())
      node->is_background_changed_ = true;
    if (node->border().HasValue())
      node->is_border_changed_ = true;
    node->is_size_changed_ = true;
  }
  node->bounds_ = new_bounds;
}

void NodeEditor::SetContentChanged(Node* node) {
  node->is_content_changed_ = true;
  ScheduleVisualUpdateIfNeeded(node);
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

void NodeEditor::SetStyle(Node* node, const css::Style& new_style) {
  if (new_style.has_display() &&
      new_style.display().is_none() != node->is_display_none_) {
    node->is_display_none_ = new_style.display().is_none();
    DidChangeChild(node->parent_);
  }

#define V(property)                                \
  if (new_style.has_##property() &&                \
      new_style.property() != node->property##_) { \
    node->property##_ = new_style.property();      \
    node->is_##property##_changed_ = true;         \
  }
  FOR_EACH_PROPERTY_CHANGES_PROPERTY(V)
#undef V

#define V(property)                                \
  if (new_style.has_##property() &&                \
      new_style.property() != node->property##_) { \
    node->property##_ = new_style.property();      \
    node->is_origin_changed_ = true;               \
  }
  FOR_EACH_PROPERTY_AFFECTS_ORIGIN(V)
#undef V

#define V(property)                                \
  if (new_style.has_##property() &&                \
      new_style.property() != node->property##_) { \
    node->property##_ = new_style.property();      \
    node->is_size_changed_ = true;                 \
  }
  FOR_EACH_PROPERTY_AFFECTS_SIZE(V)
#undef V

  if (const auto& text = node->as<TextNode>()) {
    if (new_style.has_color() && new_style.color().value() != text->color_) {
      text->color_ = new_style.color().value();
      SetContentChanged(text);
    }
  }

  if (!node->parent_)
    return;
  if (!node->is_origin_changed_ && !node->is_size_changed_)
    return;

  // Since changing size or origin affects parent's size and sibling's origin,
  // we should notify to parent.
  DidChangeChild(node->parent_);
}

void NodeEditor::SetShouldPaint(Node* node) {
  for (const auto& runner : Node::AncestorsOrSelf(*node)) {
    if (runner->should_paint_)
      return;
    runner->should_paint_ = true;
  }
}

void NodeEditor::SetViewportSize(Document* document, const FloatSize& size) {
  DCHECK(document->lifecycle_.AllowsTreeMutaions()) << document->lifecycle_;
  if (document->viewport_size_ == size)
    return;
  document->viewport_size_ = size;
  document->is_size_changed_ = true;
  ScheduleVisualUpdateIfNeeded(document);
}

void NodeEditor::WillDestroy(Node* node) {
  node->parent_ = nullptr;
}

}  // namespace visuals

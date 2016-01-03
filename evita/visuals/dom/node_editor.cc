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

Node* NodeEditor::AppendChild(ContainerNode* container, Node* new_child) {
  DCHECK(!container->document()->is_locked());
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

void NodeEditor::RemoveChild(ContainerNode* container, Node* old_child) {
  DCHECK(!container->document()->is_locked());
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
}

void NodeEditor::ScheduleVisualUpdateIfNeeded(Node* node) {
  const auto document = FindDocument(*node);
  if (!document)
    return;
}

void NodeEditor::SetContentChanged(Node* node) {
  node->is_content_changed_ = true;
  ScheduleVisualUpdateIfNeeded(node);
}

void NodeEditor::SetStyle(Element* element, const css::Style& new_style) {
  DCHECK(!element->document()->is_locked());
  if (element->inline_style_) {
    if (*element->inline_style_ == new_style)
      return;
    *element->inline_style_ == new_style;
  } else {
    element->inline_style_ = std::make_unique<css::Style>(new_style);
  }
  // TODO(eval1749): Notify inline style changes
}

void NodeEditor::SetShouldPaint(Node* node) {
  for (const auto& runner : Node::AncestorsOrSelf(*node)) {
    if (runner->should_paint_)
      return;
    runner->should_paint_ = true;
  }
}

void NodeEditor::WillDestroy(Node* node) {
  node->parent_ = nullptr;
}

}  // namespace visuals

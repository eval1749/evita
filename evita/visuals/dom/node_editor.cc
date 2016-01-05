// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/dom/node_editor.h"

#include "base/logging.h"
#include "base/observer_list.h"
#include "evita/visuals/dom/ancestors.h"
#include "evita/visuals/dom/ancestors_or_self.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/dom/text_node.h"
#include "evita/visuals/css/style.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NodeEditor
//
NodeEditor::NodeEditor() {}
NodeEditor::~NodeEditor() {}

void NodeEditor::AddClass(Element* element,
                          const base::StringPiece16& class_name) {
  const auto& it =
      std::find_if(element->class_list_.begin(), element->class_list_.end(),
                   [class_name](const base::string16& present) {
                     return class_name ==
                            base::StringPiece16(present.data(), present.size());
                   });
  if (it != element->class_list_.end())
    return;
  const auto& new_class = class_name.as_string();
  element->class_list_.emplace_back(new_class);
  const auto document = element->document();
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidAddClass(*element, new_class));
}

void NodeEditor::AppendChild(ContainerNode* container, Node* new_child) {
  const auto document = container->document_;
  DCHECK(!document->is_locked());
  if (new_child->parent_)
    RemoveChild(new_child->parent_, new_child);
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
  RegisterElementIdForSubtree(*new_child);
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidAppendChild(*container, *new_child));
}

void NodeEditor::InsertBefore(ContainerNode* container,
                              Node* new_child,
                              Node* ref_child) {
  if (!ref_child)
    return AppendChild(container, new_child);
  const auto document = container->document_;
  DCHECK(!document->is_locked());
  if (new_child->parent_)
    RemoveChild(new_child->parent_, new_child);
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->parent_);
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->next_sibling_);
  DCHECK_EQ(static_cast<ContainerNode*>(nullptr), new_child->previous_sibling_);
  const auto previous_sibling = ref_child->previous_sibling_;
  if (previous_sibling)
    previous_sibling->next_sibling_ = new_child;
  else
    container->first_child_ = new_child;
  ref_child->previous_sibling_ = new_child;
  new_child->parent_ = container;
  new_child->next_sibling_ = ref_child;
  new_child->previous_sibling_ = previous_sibling;
  RegisterElementIdForSubtree(*new_child);
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidInsertBefore(*container, *new_child, *ref_child));
}

void NodeEditor::RegisterElementIdForSubtree(const Node& node) {
  if (!node.is<ContainerNode>() || !node.InDocument())
    return;
  const auto& document = node.document();
  for (const auto& runner : Node::DescendantsOrSelf(node)) {
    if (const auto element = runner->as<Element>())
      document->RegisterElementIdIfNeeded(*element);
  }
}

void NodeEditor::RemoveChild(ContainerNode* container, Node* old_child) {
  const auto document = container->document_;
  DCHECK(!document->is_locked());
  DCHECK_EQ(container, old_child->parent_);
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidRemoveChild(*container, *old_child));
  UnregisterElementIdForSubtree(*old_child);
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
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidRemoveChild(*container, *old_child));
}

void NodeEditor::RemoveClass(Element* element,
                             const base::StringPiece16& class_name) {
  const auto& old_class = class_name.as_string();
  auto destination =
      std::find_if(element->class_list_.begin(), element->class_list_.end(),
                   [old_class](const base::string16& present) {
                     return old_class == present;
                   });
  if (destination == element->class_list_.end())
    return;
  for (auto source = std::next(destination);
       source != element->class_list_.end(); ++source) {
    *destination = *source;
    ++destination;
  }
  element->class_list_.pop_back();
  const auto document = element->document();
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidRemoveClass(*element, old_class));
}

void NodeEditor::SetStyle(Element* element, const css::Style& new_style) {
  const auto document = element->document();
  DCHECK(!document->is_locked());
  if (element->inline_style_) {
    if (*element->inline_style_ == new_style)
      return;
    auto old_style = std::move(element->inline_style_);
    element->inline_style_.reset(new css::Style(new_style));
    FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                      DidChangeInlineStyle(*element, old_style.get()));
    return;
  }
  element->inline_style_ = std::make_unique<css::Style>(new_style);
  ++document->version_;
  FOR_EACH_OBSERVER(DocumentObserver, document->observers_,
                    DidChangeInlineStyle(*element, nullptr));
}

void NodeEditor::UnregisterElementIdForSubtree(const Node& node) {
  if (!node.is<ContainerNode>() || !node.InDocument())
    return;
  const auto& document = node.document();
  for (const auto& runner : Node::DescendantsOrSelf(node)) {
    if (const auto element = runner->as<Element>())
      document->UnregisterElementIdIfNeeded(*element);
  }
}

void NodeEditor::WillDestroy(Node* node) {
  DCHECK(!node->document()->is_locked());
  node->parent_ = nullptr;
}

}  // namespace visuals

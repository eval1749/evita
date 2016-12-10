// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/node_editor.h"

#include "joana/public/ast/container_node.h"

namespace joana {
namespace ast {

NodeEditor::NodeEditor() = default;
NodeEditor::~NodeEditor() = default;

NodeEditor& NodeEditor::AppendChild(ContainerNode* container, Node* new_child) {
  DCHECK(!container->Contains(*new_child)) << *container << ' ' << *new_child;
  DCHECK_NE(container->last_child_, new_child);
  if (new_child->parent_)
    RemoveChild(new_child->parent_, new_child);
  DCHECK(!new_child->parent_);
  DCHECK(!new_child->next_sibling_);
  DCHECK(!new_child->previous_sibling_);
  auto* const last_child = container->last_child_;
  if (last_child) {
    last_child->next_sibling_ = new_child;
    new_child->previous_sibling_ = last_child;
  } else {
    container->first_child_ = new_child;
  }
  new_child->parent_ = container;
  container->last_child_ = new_child;
  return *this;
}

NodeEditor& NodeEditor::InsertBefore(ContainerNode* container,
                                     Node* new_child,
                                     Node* ref_child) {
  DCHECK_NE(new_child, ref_child);
  if (!ref_child)
    return AppendChild(container, new_child);
  DCHECK(!container->Contains(*new_child)) << *container << ' ' << *new_child;
  DCHECK_EQ(container, ref_child->parent_);
  if (new_child->parent_)
    RemoveChild(new_child->parent_, new_child);
  DCHECK(!new_child->parent_);
  DCHECK(!new_child->next_sibling_);
  DCHECK(!new_child->previous_sibling_);
  auto* const previous = ref_child->previous_sibling_;
  if (previous)
    previous->next_sibling_ = new_child;
  else
    container->first_child_ = new_child;
  new_child->next_sibling_ = ref_child;
  new_child->previous_sibling_ = previous;
  new_child->parent_ = container;
  ref_child->previous_sibling_ = new_child;
  return *this;
}

NodeEditor& NodeEditor::RemoveChild(ContainerNode* container, Node* old_child) {
  DCHECK_EQ(container, old_child->parent_);
  auto* const next = old_child->next_sibling_;
  auto* const previous = old_child->previous_sibling_;
  if (next)
    next->previous_sibling_ = previous;
  else
    container->last_child_ = previous;
  if (previous)
    previous->next_sibling_ = next;
  else
    container->first_child_ = next;
  old_child->next_sibling_ = nullptr;
  old_child->previous_sibling_ = nullptr;
  old_child->parent_ = nullptr;
  return *this;
}

NodeEditor& NodeEditor::RemoveNode(Node* old_child) {
  DCHECK(old_child->parent_);
  return RemoveChild(old_child->parent_, old_child);
}

NodeEditor& NodeEditor::ReplaceChild(ContainerNode* container,
                                     Node* new_child,
                                     Node* old_child) {
  DCHECK_NE(new_child, old_child);
  DCHECK_EQ(container, old_child->parent_);
  if (new_child->parent_)
    RemoveChild(container, new_child);
  auto* const next = old_child->next_sibling_;
  auto* const previous = old_child->previous_sibling_;
  if (next)
    next->previous_sibling_ = new_child;
  else
    container->last_child_ = new_child;
  if (previous)
    previous->next_sibling_ = new_child;
  else
    container->first_child_ = new_child;
  old_child->next_sibling_ = nullptr;
  old_child->previous_sibling_ = nullptr;
  old_child->parent_ = nullptr;
  new_child->next_sibling_ = next;
  new_child->previous_sibling_ = previous;
  new_child->parent_ = container;
  return *this;
}

}  // namespace ast
}  // namespace joana

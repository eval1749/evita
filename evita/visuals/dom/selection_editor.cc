// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/selection_editor.h"

#include "base/logging.h"
#include "evita/visuals/dom/selection.h"
#include "evita/visuals/dom/text.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// SelectionEditor
//
SelectionEditor::SelectionEditor() {}
SelectionEditor::~SelectionEditor() {}

void SelectionEditor::Clear(Selection* selection) {
  selection->anchor_node_ = selection->focus_node_ = nullptr;
  selection->anchor_offset_ = selection->focus_offset_ = 0;
}

void SelectionEditor::Collapse(Selection* selection, Node* node, int offset) {
  DCHECK(node->is<Text>()) << *node;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), node->as<Text>()->data().size());
  selection->anchor_node_ = selection->focus_node_ = node;
  selection->anchor_offset_ = selection->focus_offset_ = offset;
}

void SelectionEditor::ExtendTo(Selection* selection, Node* node, int offset) {
  DCHECK(!selection->is_none());
  DCHECK(node->is<Text>()) << *node;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), node->as<Text>()->data().size());
  selection->focus_node_ = node;
  selection->focus_offset_ = offset;
}

}  // namespace visuals

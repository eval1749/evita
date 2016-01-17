// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/selection_model.h"

#include "base/logging.h"
#include "evita/visuals/dom/text.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// SelectionModel
//
SelectionModel::SelectionModel(const SelectionModel& other)
    : anchor_node_(other.anchor_node_),
      anchor_offset_(other.anchor_offset_),
      focus_node_(other.focus_node_),
      focus_offset_(other.focus_offset_) {}

SelectionModel::SelectionModel() {}
SelectionModel::~SelectionModel() {}

bool SelectionModel::operator==(const SelectionModel& other) const {
  if (this == &other)
    return true;
  if (!anchor_node_)
    return !other.anchor_node_;
  if (is_caret()) {
    if (!other.is_caret())
      return false;
    return anchor_node_ == other.anchor_node_ &&
           anchor_offset_ == other.anchor_offset_;
  }
  if (!other.is_range())
    return false;
  return anchor_node_ == other.anchor_node_ &&
         anchor_offset_ == other.anchor_offset_ &&
         focus_node_ == other.focus_node_ &&
         focus_offset_ == other.focus_offset_;
}

bool SelectionModel::operator!=(const SelectionModel& other) const {
  return !operator==(other);
}

const Node& SelectionModel::anchor_node() const {
  DCHECK(is_range());
  return *anchor_node_;
}

int SelectionModel::anchor_offset() const {
  DCHECK(is_range());
  return anchor_offset_;
}

const Node& SelectionModel::focus_node() const {
  DCHECK(!is_none());
  return *focus_node_;
}

int SelectionModel::focus_offset() const {
  DCHECK(!is_none());
  return focus_offset_;
}

bool SelectionModel::is_caret() const {
  if (is_none())
    return false;
  return anchor_node_ == focus_node_ && anchor_offset_ == focus_offset_;
}

bool SelectionModel::is_none() const {
  return !anchor_node_;
}

bool SelectionModel::is_range() const {
  if (is_none())
    return false;
  if (anchor_node_ != focus_node_)
    return true;
  return anchor_offset_ != focus_offset_;
}

void SelectionModel::Clear() {
  anchor_node_ = focus_node_ = nullptr;
  anchor_offset_ = focus_offset_ = 0;
}

void SelectionModel::Collapse(Node* node, int offset) {
  DCHECK(node->is<Text>()) << *node;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), node->as<Text>()->data().size());
  anchor_node_ = focus_node_ = node;
  anchor_offset_ = focus_offset_ = offset;
}

void SelectionModel::ExtendTo(Node* node, int offset) {
  DCHECK(node->is<Text>()) << *node;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), node->as<Text>()->data().size());
  focus_node_ = node;
  focus_offset_ = offset;
}

void SelectionModel::WillRemoveChild(const ContainerNode& parent,
                                     const Node& child) {
  if (is_none())
    return;
  if (is_caret()) {
    if (child != focus_node_)
      return;
    return Clear();
  }
  if (child == anchor_node_)
    return Collapse(focus_node_, focus_offset_);
  if (child != focus_node_)
    return;
  return Collapse(anchor_node_, anchor_offset_);
}

}  // namespace visuals

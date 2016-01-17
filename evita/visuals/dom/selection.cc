// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/selection.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(const Selection& other)
    : anchor_node_(other.anchor_node_),
      anchor_offset_(other.anchor_offset_),
      focus_node_(other.focus_node_),
      focus_offset_(other.focus_offset_) {}

Selection::Selection() {}
Selection::~Selection() {}

bool Selection::operator==(const Selection& other) const {
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

bool Selection::operator!=(const Selection& other) const {
  return !operator==(other);
}

bool Selection::is_caret() const {
  if (is_none())
    return false;
  return anchor_node_ == focus_node_ && anchor_offset_ == focus_offset_;
}

bool Selection::is_none() const {
  return !anchor_node_;
}

bool Selection::is_range() const {
  if (is_none())
    return false;
  if (anchor_node_ != focus_node_)
    return true;
  return anchor_offset_ != focus_offset_;
}

}  // namespace visuals

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_selection.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxSelection
//
BoxSelection::BoxSelection(const BoxSelection& other)
    : anchor_box_(other.anchor_box_),
      anchor_offset_(other.anchor_offset_),
      caret_color_(other.caret_color_),
      caret_shape_(other.caret_shape_),
      focus_box_(other.focus_box_),
      focus_offset_(other.focus_offset_),
      selection_color_(other.selection_color_) {}

BoxSelection::BoxSelection() {}
BoxSelection::~BoxSelection() {}

bool BoxSelection::operator==(const BoxSelection& other) const {
  if (this == &other)
    return true;
  if (!anchor_box_)
    return !other.anchor_box_;
  if (is_caret()) {
    if (!other.is_caret())
      return false;
    return anchor_box_ == other.anchor_box_ &&
           anchor_offset_ == other.anchor_offset_ &&
           caret_color_ == other.caret_color_ &&
           caret_shape_ == other.caret_shape_;
  }
  if (!other.is_range())
    return false;
  return anchor_box_ == other.anchor_box_ &&
         anchor_offset_ == other.anchor_offset_ &&
         focus_box_ == other.focus_box_ &&
         focus_offset_ == other.focus_offset_ &&
         selection_color_ == other.selection_color_;
}

bool BoxSelection::operator!=(const BoxSelection& other) const {
  return !operator==(other);
}

bool BoxSelection::is_caret() const {
  if (is_none())
    return false;
  return anchor_box_ == focus_box_ && anchor_offset_ == focus_offset_;
}

bool BoxSelection::is_none() const {
  return !anchor_box_;
}

bool BoxSelection::is_range() const {
  if (is_none())
    return false;
  if (anchor_box_ != focus_box_)
    return true;
  return anchor_offset_ != focus_offset_;
}

}  // namespace visuals

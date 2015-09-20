// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/text_field_selection.h"

#include <algorithm>

#include "evita/dom/forms/text_field_control.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextFieldSelection
//
TextFieldSelection::TextFieldSelection(TextFieldControl* control)
    : ScriptableBase(),
      control_(control),
      anchor_offset_(0),
      focus_offset_(0) {}

TextFieldSelection::~TextFieldSelection() {}

void TextFieldSelection::set_anchor_offset(int anchor_offset) {
  auto const new_anchor_offset = NormalizeOffset(anchor_offset);
  if (anchor_offset_ == new_anchor_offset)
    return;
  anchor_offset_ = new_anchor_offset;
  control_->DidChangeSelection();
}

void TextFieldSelection::set_focus_offset(int focus_offset) {
  auto const new_focus_offset = NormalizeOffset(focus_offset);
  if (focus_offset_ == new_focus_offset)
    return;
  focus_offset_ = new_focus_offset;
  control_->DidChangeSelection();
}

void TextFieldSelection::DidChangeValue() {
  anchor_offset_ = NormalizeOffset(anchor_offset_);
  focus_offset_ = NormalizeOffset(focus_offset_);
}

int TextFieldSelection::NormalizeOffset(int offset) const {
  return std::max(0, std::min(offset, static_cast<int>(control_->length())));
}

}  // namespace dom

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_selection_editor.h"

#include "base/logging.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/layout/box_selection.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxSelectionEditor
//
BoxSelectionEditor::BoxSelectionEditor() {}
BoxSelectionEditor::~BoxSelectionEditor() {}

void BoxSelectionEditor::Clear(BoxSelection* selection) {
  selection->anchor_box_ = selection->focus_box_ = nullptr;
  selection->anchor_offset_ = selection->focus_offset_ = 0;
}

void BoxSelectionEditor::Collapse(BoxSelection* selection,
                                  Box* box,
                                  int offset) {
  DCHECK(box->is<TextBox>()) << *box;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), box->as<TextBox>()->data().size());
  selection->anchor_box_ = selection->focus_box_ = box;
  selection->anchor_offset_ = selection->focus_offset_ = offset;
}

void BoxSelectionEditor::ExtendTo(BoxSelection* selection,
                                  Box* box,
                                  int offset) {
  DCHECK(!selection->is_none());
  DCHECK(box->is<TextBox>()) << *box;
  DCHECK_GE(offset, 0);
  DCHECK_LE(static_cast<size_t>(offset), box->as<TextBox>()->data().size());
  selection->focus_box_ = box;
  selection->focus_offset_ = offset;
}

void BoxSelectionEditor::SetCaretColor(BoxSelection* selection,
                                       const FloatColor& color) {
  selection->caret_color_ = color;
}

void BoxSelectionEditor::SetCaretShape(BoxSelection* selection,
                                       const css::CaretShape& shape) {
  selection->caret_shape_ = shape;
}

void BoxSelectionEditor::SetSelectionColor(BoxSelection* selection,
                                           const FloatColor& color) {
  selection->selection_color_ = color;
}

}  // namespace visuals

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/text_field_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_paint_info.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/visuals/fonts/text_format.h"
#include "evita/visuals/fonts/text_layout.h"
#include "evita/visuals/geometry/float_point.h"
#include "evita/visuals/geometry/float_size.h"

namespace dom {

using FloatPoint = visuals::FloatPoint;
using FloatSize = visuals::FloatSize;

namespace {
base::string16 EnsureSingleLine(const base::string16& text) {
  auto last_char = 0;
  for (auto index = 0u; index < text.size(); ++index) {
    if (text[index] == '\n') {
      return last_char == '\r' ? text.substr(0, index - 1)
                               : text.substr(0, index);
    }
    last_char = text[index];
  }
  return text;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl

//
TextFieldControl::TextFieldControl()
    : selection_(new TextFieldSelection(this)) {}

TextFieldControl::~TextFieldControl() {}

void TextFieldControl::set_scroll_left(float new_left) {
  if (scroll_left_ == new_left)
    return;
  scroll_left_ = new_left;
  NotifyControlChange();
}

// This function is used for implementing setter of |value| property.
// Caller should dispatch "input" event.
void TextFieldControl::set_value(const base::string16& new_raw_value) {
  auto const new_value = EnsureSingleLine(new_raw_value);
  if (value_ == new_value)
    return;
  value_ = new_value;
  selection_->DidChangeValue();
  NotifyControlChange();
}

void TextFieldControl::DidChangeSelection() {
  NotifyControlChange();
}

int TextFieldControl::MapPointToOffset(int x, int y) const {
  const auto kHuge = 1.0e6f;
  const auto& layout = std::make_unique<visuals::TextLayout>(
      form()->GetTextFormat(), value_, visuals::FloatSize(kHuge, kHuge));
  const auto& point = FloatPoint(x, y) + FloatSize(scroll_left_, 0);
  return static_cast<int>(layout->HitTestPoint(point));
}

// FormControl
std::unique_ptr<domapi::FormControl> TextFieldControl::Paint(
    const FormPaintInfo& paint_info) const {
  auto state = ComputeState(paint_info);
  domapi::TextField::Selection selection;
  if (state.focused()) {
    if (selection_->collapsed()) {
      selection.Collapse(selection_->focus_offset());
    } else {
      selection.Collapse(selection_->anchor_offset());
      selection.Extend(selection_->focus_offset());
    }
    selection.set_caret_shape(paint_info.caret_shape());
  }
  return std::make_unique<domapi::TextField>(event_target_id(), bounds(), state,
                                             selection, scroll_left_, value_);
}

}  // namespace dom

// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/text_selection.h"

#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/windows/text_window.h"
#include "evita/text/selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(TextWindow* text_window, Range* range)
    : ScriptableBase(text_window, range->document()),
      text_selection_(new text::Selection(range->text_range())),
      range_(new Range(range->document(), text_selection_->range())) {
}

TextSelection::~TextSelection() {
}

Posn TextSelection::anchor_offset() const {
  return start_is_active() ? range_->end() : range_->start();
}

Posn TextSelection::focus_offset() const {
  return start_is_active() ? range_->start() : range_->end();
}

bool TextSelection::start_is_active() const {
  return text_selection_->IsStartActive();
}

void TextSelection::set_start_is_active(bool start_is_active) {
  text_selection_->SetStartIsActive(start_is_active);
}

}  // namespace dom

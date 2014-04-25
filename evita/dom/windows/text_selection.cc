// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/text_selection.h"

#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/windows/text_window.h"
#include "evita/vi_Selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(TextWindow* text_window, Range* range)
    : ScriptableBase(text_window, range->document()),
      range_(new Range(range->document(), range->start(), range->end())),
      view_selection_(new ::Selection(range_->text_range())) {
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
  return view_selection_->IsStartActive();
}

void TextSelection::set_start_is_active(bool start_is_active) {
  view_selection_->SetStartIsActive(start_is_active);
}

}  // namespace dom

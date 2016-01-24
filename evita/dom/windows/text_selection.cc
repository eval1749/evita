// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/text_selection.h"

#include "evita/bindings/v8_glue_UiEventInit.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_range.h"
#include "evita/dom/windows/text_window.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "evita/text/selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(TextWindow* text_window, TextRange* range)
    : document_(range->document()),
      text_selection_(new text::Selection(range->text_range())),
      range_(new TextRange(range->document(), text_selection_->range())),
      window_(text_window) {
  text_selection_->AddObserver(this);
}

TextSelection::~TextSelection() {
  text_selection_->RemoveObserver(this);
}

text::Offset TextSelection::anchor_offset() const {
  return start_is_active() ? range_->text_range()->end()
                           : range_->text_range()->start();
}

int TextSelection::anchor_offset_value() const {
  return anchor_offset().value();
}

text::Offset TextSelection::focus_offset() const {
  return start_is_active() ? range_->text_range()->start()
                           : range_->text_range()->end();
}

int TextSelection::focus_offset_value() const {
  return focus_offset().value();
}

bool TextSelection::start_is_active() const {
  return text_selection_->IsStartActive();
}

void TextSelection::set_start_is_active(bool start_is_active) {
  text_selection_->SetStartIsActive(start_is_active);
}

// text::SelectionChangeObserver
void TextSelection::DidChangeSelection() {
  UiEventInit ui_event_init;
  ui_event_init.set_detail(0);
  auto const target = window();
  ui_event_init.set_view(target);
  auto event = new UiEvent(L"selectionchange", ui_event_init);
  if (!target->DispatchEvent(event))
    return;
  ScriptHost::instance()->CallClassEventHandler(target, event);
}

}  // namespace dom

// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/text_field_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"

namespace dom {

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

v8::Local<v8::Promise> TextFieldControl::MapPointToOffset(float x,
                                                          float y) const {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::ViewDelegate::MapTextFieldPointToOffset,
                 base::Unretained(ScriptHost::instance()->view_delegate()),
                 event_target_id(), x, y));
}

}  // namespace dom

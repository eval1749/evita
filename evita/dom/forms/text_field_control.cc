// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/text_field_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

namespace {
base::string16 EnsureSingleLine(const base::string16& text) {
  auto last_char = 0;
  for (auto index = 0u; index < text.size(); ++index){
    if (text[index] == '\n') {
      return last_char == '\r' ? text.substr(0, index - 1) :
                                 text.substr(0, index);
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
    : selection_(new TextFieldSelection(this)) {
}

TextFieldControl::~TextFieldControl() {
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

int TextFieldControl::MapPointToOffset(float x, float y) const {
  return ScriptHost::instance()->view_delegate()->MapPointToPosition(
      event_target_id(), x, y);
}

}  // namespace dom

// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/document_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

DocumentEventInit::DocumentEventInit(Window* view) : view_(view){
  set_bubbles(true);
  set_cancelable(false);
}

DocumentEventInit::DocumentEventInit() : DocumentEventInit(nullptr) {
}

DocumentEventInit::~DocumentEventInit() {
}

Dictionary::HandleResult DocumentEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::view.Get(isolate()))) {
    v8_glue::Nullable<Window> maybe_view;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_view))
      return HandleResult::CanNotConvert;
    view_ = maybe_view;
    return HandleResult::Succeeded;
  }

  return EventInit::HandleKeyValue(key, value);
}

}  // namespace dom

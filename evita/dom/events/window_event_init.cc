// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

WindowEventInit::WindowEventInit() {
}

WindowEventInit::~WindowEventInit() {
}

Dictionary::HandleResult WindowEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::sourceWindow.Get(isolate()))) {
    v8_glue::Nullable<Window> maybe_window;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_window))
      return HandleResult::CanNotConvert;
    source_window_ = maybe_window;
    return HandleResult::Succeeded;
  }

  return EventInit::HandleKeyValue(key, value);
}

}  // namespace dom

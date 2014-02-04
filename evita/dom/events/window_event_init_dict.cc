// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event_init_dict.h"

#include "evita/dom/converter.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

WindowEventInitDict::WindowEventInitDict() {
}

WindowEventInitDict::~WindowEventInitDict() {
}

InitDict::HandleResult WindowEventInitDict::HandleKeyValue(
    v8::Handle<v8::String> key, v8::Handle<v8::Value> value) {
  if (EqualNames(key, v8String::sourceWindow)) {
    v8_glue::Nullable<Window> maybe_window;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_window)
      return HandleResult::CanNotConvert;
    window_ = maybe_window.value;
    return HandleResult::Succeeded;
  }

  return EventInitDict::HaneleKeyValue(key, value);
}

}  // namespace dom

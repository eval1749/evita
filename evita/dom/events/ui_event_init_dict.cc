// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/ui_event_init_dict.h"

#include "evita/dom/converter.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

UiEventInitDict::UiEventInitDict() : detail_(0) {
}

UiEventInitDict::~UiEventInitDict() {
}

InitDict::HandleResult UiEventInitDict::HandleKeyValue(
    v8::Handle<v8::String> key, v8::Handle<v8::Value> value) {
  if (EqualNames(key, v8Strings::detail)) {
    return gin::ConvertFromV8(isolate(), value, &detail_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (EqualNames(key, v8Strings::view)) {
    v8_glue::Nullable<Window> maybe_view;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_view))
      return HandleResult::CanNotConvert;
    view_ = maybe_view;
    return HandleResult::Succeeded;
  }

  return EventInitDict::HandleKeyValue(key, value);
}

}  // namespace dom

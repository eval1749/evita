// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/focus_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

FocusEventInit::FocusEventInit() {
  set_bubbles(false);
  set_cancelable(false);
}

FocusEventInit::~FocusEventInit() {
}

Dictionary::HandleResult FocusEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {

  if (key->Equals(v8Strings::relatedTarget.Get(isolate()))) {
    v8_glue::Nullable<EventTarget> maybe_related_target;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_related_target))
      return HandleResult::CanNotConvert;
    related_target_ = maybe_related_target;
    return HandleResult::Succeeded;
  }

  return UiEventInit::HandleKeyValue(key, value);
}

}  // namespace dom

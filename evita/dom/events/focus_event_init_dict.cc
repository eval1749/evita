// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/focus_event_init_dict.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

FocusEventInitDict::FocusEventInitDict() {
}

FocusEventInitDict::~FocusEventInitDict() {
}

InitDict::HandleResult FocusEventInitDict::HandleKeyValue(
    v8::Handle<v8::String> key, v8::Handle<v8::Value> value) {

  if (EqualNames(key, v8Strings::relatedTarget)) {
    v8_glue::Nullable<EventTarget> maybe_related_target;
    if (!gin::ConvertFromV8(isolate(), value, &maybe_related_target))
      return HandleResult::CanNotConvert;
    related_target_ = maybe_related_target;
    return HandleResult::Succeeded;
  }

  return UiEventInitDict::HandleKeyValue(key, value);
}

}  // namespace dom

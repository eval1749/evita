// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

EventInit::EventInit() : bubbles_(false), cancelable_(false) {
}

EventInit::~EventInit() {
}

InitDict::HandleResult EventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::bubbles.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &bubbles_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::cancelable.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &cancelable_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  return HandleResult::NotFound;
}

}  // namespace dom

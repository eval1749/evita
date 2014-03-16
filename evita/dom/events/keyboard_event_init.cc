// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/keyboard_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

namespace {
int ConvertClickCount(const domapi::KeyboardEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return 1;
  if (event.event_type == domapi::EventType::DblClick)
    return 2;
  return 0;
}
}  // namespace

KeyboardEventInit::KeyboardEventInit(const domapi::KeyboardEvent& event)
    : alt_key_(event.alt_key), ctrl_key_(event.control_key),
      key_code_(event.key_code), location_(event.location),
      meta_key_(false), repeat_(event.repeat), shift_key_(event.shift_key) {
  set_bubbles(true);
  set_cancelable(true);
}

KeyboardEventInit::KeyboardEventInit()
    : alt_key_(false), ctrl_key_(false), key_code_(0), location_(0),
      meta_key_(false), repeat_(false), shift_key_(false) {
  set_bubbles(true);
  set_cancelable(true);
}

KeyboardEventInit::~KeyboardEventInit() {
}

Dictionary::HandleResult KeyboardEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {

  if (key->Equals(v8Strings::altKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &alt_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::ctrlKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &ctrl_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::code.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &key_code_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::location.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &location_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::metaKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &meta_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::repeat.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &repeat_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::shiftKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &shift_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }

  return UiEventInit::HandleKeyValue(key, value);
}

}  // namespace dom
